#include "editor_scene/EditorSceneCompression.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <new>
#include <utility>

namespace
{
constexpr int WindowSize = 4096;
constexpr int LookaheadSize = 60;
constexpr int Threshold = 2;
constexpr int CharacterCount = 256 - Threshold + LookaheadSize;
constexpr int TreeSize = CharacterCount * 2 - 1;
constexpr int Root = TreeSize - 1;
constexpr unsigned MaximumFrequency = 0x4000;

constexpr std::array<std::uint8_t, 64> PositionLengths{{
    3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}};

constexpr std::array<std::uint8_t, 64> PositionCodes{{
    0x00, 0x20, 0x30, 0x40, 0x50, 0x58, 0x60, 0x68,
    0x70, 0x78, 0x80, 0x88, 0x90, 0x94, 0x98, 0x9c,
    0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc,
    0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce,
    0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde,
    0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff}};

bool SetReason(std::string* reason, const char* message)
{
    if (reason)
        *reason = message;
    return false;
}

class BitReader
{
public:
    BitReader(const std::uint8_t* data, std::size_t size) :
        data_(data), size_(size)
    {
    }

    bool ReadBit(unsigned& value)
    {
        if (bitPosition_ >= size_ * 8u)
            return false;
        const std::size_t byte = bitPosition_ / 8u;
        const unsigned bit = 7u - static_cast<unsigned>(bitPosition_ % 8u);
        value = (data_[byte] >> bit) & 1u;
        ++bitPosition_;
        return true;
    }

    bool ReadBits(unsigned count, unsigned& value)
    {
        value = 0;
        for (unsigned index = 0; index < count; ++index)
        {
            unsigned bit = 0;
            if (!ReadBit(bit))
                return false;
            value = (value << 1u) | bit;
        }
        return true;
    }

    bool HasOnlyZeroPadding() const
    {
        for (std::size_t position = bitPosition_; position < size_ * 8u;
            ++position)
        {
            const std::size_t byte = position / 8u;
            const unsigned bit = 7u - static_cast<unsigned>(position % 8u);
            if (((data_[byte] >> bit) & 1u) != 0)
                return false;
        }
        return true;
    }

    std::size_t RemainingBits() const
    {
        return size_ * 8u - bitPosition_;
    }

private:
    const std::uint8_t* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t bitPosition_ = 0;
};

class Decoder
{
public:
    explicit Decoder(BitReader& input) : input_(input)
    {
        StartHuffman();
        textBuffer_.fill(0x20);
    }

    bool Decode(std::vector<std::uint8_t>& output, std::size_t expectedSize,
        std::string* reason)
    {
        int ringPosition = WindowSize - LookaheadSize;
        while (output.size() < expectedSize)
        {
            int character = 0;
            if (!DecodeCharacter(character))
                return SetReason(reason, "truncated LZHUF character stream");
            if (character < 256)
            {
                output.push_back(static_cast<std::uint8_t>(character));
                textBuffer_[ringPosition] = static_cast<std::uint8_t>(character);
                ringPosition = (ringPosition + 1) & (WindowSize - 1);
                continue;
            }

            int position = 0;
            if (!DecodePosition(position))
                return SetReason(reason, "truncated LZHUF back-reference");
            const int source = (ringPosition - position - 1) &
                (WindowSize - 1);
            const int length = character - 255 + Threshold;
            const std::size_t remaining = expectedSize - output.size();
            if (static_cast<std::size_t>(length) > remaining)
                return SetReason(reason,
                    "LZHUF back-reference exceeds declared output size");
            for (int index = 0; index < length; ++index)
            {
                const std::uint8_t value =
                    textBuffer_[(source + index) & (WindowSize - 1)];
                output.push_back(value);
                textBuffer_[ringPosition] = value;
                ringPosition = (ringPosition + 1) & (WindowSize - 1);
            }
        }
        return true;
    }

private:
    void StartHuffman()
    {
        for (int index = 0; index < CharacterCount; ++index)
        {
            frequencies_[index] = 1;
            children_[index] = index + TreeSize;
            parents_[index + TreeSize] = index;
        }
        int child = 0;
        for (int parent = CharacterCount; parent <= Root; ++parent)
        {
            frequencies_[parent] = frequencies_[child] +
                frequencies_[child + 1];
            children_[parent] = child;
            parents_[child] = parent;
            parents_[child + 1] = parent;
            child += 2;
        }
        frequencies_[TreeSize] = 0xffff;
        parents_[Root] = 0;
    }

    void Reconstruct()
    {
        int destination = 0;
        for (int index = 0; index < TreeSize; ++index)
        {
            if (children_[index] >= TreeSize)
            {
                frequencies_[destination] = (frequencies_[index] + 1) / 2;
                children_[destination] = children_[index];
                ++destination;
            }
        }
        int child = 0;
        for (int parent = CharacterCount; parent < TreeSize;
            child += 2, ++parent)
        {
            const unsigned frequency = frequencies_[child] +
                frequencies_[child + 1];
            int insertion = parent - 1;
            while (insertion >= 0 && frequency < frequencies_[insertion])
                --insertion;
            ++insertion;
            const std::size_t count = static_cast<std::size_t>(parent - insertion);
            std::memmove(&frequencies_[insertion + 1],
                &frequencies_[insertion], count * sizeof(frequencies_[0]));
            frequencies_[insertion] = frequency;
            std::memmove(&children_[insertion + 1], &children_[insertion],
                count * sizeof(children_[0]));
            children_[insertion] = child;
        }
        for (int index = 0; index < TreeSize; ++index)
        {
            const int childIndex = children_[index];
            if (childIndex >= TreeSize)
                parents_[childIndex] = index;
            else
            {
                parents_[childIndex] = index;
                parents_[childIndex + 1] = index;
            }
        }
    }

    void Update(int character)
    {
        if (frequencies_[Root] == MaximumFrequency)
            Reconstruct();
        int node = parents_[character + TreeSize];
        do
        {
            const unsigned frequency = ++frequencies_[node];
            int exchange = node + 1;
            if (frequency > frequencies_[exchange])
            {
                while (frequency > frequencies_[++exchange]) {}
                --exchange;
                frequencies_[node] = frequencies_[exchange];
                frequencies_[exchange] = frequency;

                int child = children_[node];
                parents_[child] = exchange;
                if (child < TreeSize)
                    parents_[child + 1] = exchange;
                const int other = children_[exchange];
                children_[exchange] = child;
                parents_[other] = node;
                if (other < TreeSize)
                    parents_[other + 1] = node;
                children_[node] = other;
                node = exchange;
            }
        } while ((node = parents_[node]) != 0);
    }

    bool DecodeCharacter(int& character)
    {
        int node = children_[Root];
        while (node < TreeSize)
        {
            unsigned bit = 0;
            if (!input_.ReadBit(bit))
                return false;
            node = children_[node + static_cast<int>(bit)];
        }
        character = node - TreeSize;
        Update(character);
        return true;
    }

    bool DecodePosition(int& position)
    {
        unsigned prefix = 0;
        if (!input_.ReadBits(8, prefix))
            return false;
        unsigned upper = 0;
        unsigned length = 0;
        for (; upper < PositionCodes.size(); ++upper)
        {
            length = PositionLengths[upper];
            const unsigned mask = 0xffu << (8u - length);
            if ((prefix & mask) == (PositionCodes[upper] & mask))
                break;
        }
        if (upper == PositionCodes.size())
            return false;
        unsigned suffix = 0;
        if (!input_.ReadBits(length - 2u, suffix))
            return false;
        const unsigned combined = (prefix << (length - 2u)) | suffix;
        position = static_cast<int>((upper << 6u) | (combined & 0x3fu));
        return true;
    }

    BitReader& input_;
    std::array<std::uint8_t, WindowSize + LookaheadSize> textBuffer_{};
    std::array<unsigned, TreeSize + 1> frequencies_{};
    std::array<int, TreeSize + CharacterCount + 1> parents_{};
    std::array<int, TreeSize> children_{};
};
}

const char* HistoricalSceneCompressionAlgorithm()
{
    return "X-Ray LZHUF (LZSS + adaptive Huffman)";
}

bool DecompressHistoricalSceneChunk(const std::uint8_t* compressed,
    std::size_t compressedSize, std::vector<std::uint8_t>& output,
    const EditorSceneDecompressionLimits& limits, std::string* reason)
{
    if (compressedSize > limits.maximumCompressedBytes)
        return SetReason(reason, "compressed chunk exceeds byte limit");
    if (compressedSize < sizeof(std::uint32_t) || !compressed)
        return SetReason(reason, "compressed chunk lacks LZHUF size header");
    const std::uint32_t declaredSize =
        static_cast<std::uint32_t>(compressed[0]) |
        (static_cast<std::uint32_t>(compressed[1]) << 8u) |
        (static_cast<std::uint32_t>(compressed[2]) << 16u) |
        (static_cast<std::uint32_t>(compressed[3]) << 24u);
    const std::size_t expectedSize = declaredSize;
    if (expectedSize > limits.maximumDecompressedBytes)
        return SetReason(reason, "decompressed chunk exceeds byte limit");
    if (limits.maximumExpansionRatio <= 0.0)
        return SetReason(reason, "decompression expansion ratio is invalid");
    const long double allowed = static_cast<long double>(compressedSize) *
        static_cast<long double>(limits.maximumExpansionRatio);
    if (static_cast<long double>(expectedSize) > allowed)
        return SetReason(reason, "decompressed chunk exceeds expansion ratio");

    std::vector<std::uint8_t> candidate;
    try
    {
        candidate.reserve(expectedSize);
    }
    catch (const std::bad_alloc&)
    {
        return SetReason(reason, "decompressed chunk allocation failed");
    }
    if (expectedSize == 0)
    {
        if (compressedSize != sizeof(std::uint32_t))
            return SetReason(reason, "empty LZHUF stream has trailing data");
    }
    else
    {
        BitReader bits(compressed + sizeof(std::uint32_t),
            compressedSize - sizeof(std::uint32_t));
        Decoder decoder(bits);
        if (!decoder.Decode(candidate, expectedSize, reason))
            return false;
        if (bits.RemainingBits() >= 8u || !bits.HasOnlyZeroPadding())
            return SetReason(reason, "LZHUF stream has trailing data");
    }
    output.swap(candidate);
    if (reason)
        reason->clear();
    return true;
}
