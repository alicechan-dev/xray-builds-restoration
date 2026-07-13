#ifndef XR_WX_SDK_EDITOR_EDITOR_BINARY_READER_H
#define XR_WX_SDK_EDITOR_EDITOR_BINARY_READER_H

#include <cstddef>
#include <cstdint>
#include <string>

class EditorBinaryReader
{
public:
    EditorBinaryReader() = default;
    EditorBinaryReader(const std::uint8_t* data, std::size_t size,
        std::size_t baseOffset = 0);

    std::size_t Position() const { return position_; }
    std::size_t AbsoluteOffset() const { return baseOffset_ + position_; }
    std::size_t Size() const { return size_; }
    std::size_t Remaining() const { return size_ - position_; }
    bool Empty() const { return Remaining() == 0; }
    const std::string& Error() const { return error_; }

    bool Seek(std::size_t position);
    bool Skip(std::size_t size);
    bool ReadU16(std::uint16_t& value);
    bool ReadU32(std::uint32_t& value);
    bool ReadFloat(float& value);
    bool ReadBytes(void* destination, std::size_t size);
    bool ReadCString(std::string& value, std::size_t maximumLength);
    bool Slice(std::size_t size, EditorBinaryReader& result);

private:
    bool Fail(const char* message);

    const std::uint8_t* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t position_ = 0;
    std::size_t baseOffset_ = 0;
    std::string error_;
};

#endif
