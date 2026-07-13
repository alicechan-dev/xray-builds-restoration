#include "editor_scene/EditorBinaryReader.h"

#include <cstring>
#include <limits>

EditorBinaryReader::EditorBinaryReader(const std::uint8_t* data,
    std::size_t size, std::size_t baseOffset) :
    data_(data), size_(size), baseOffset_(baseOffset)
{
}

bool EditorBinaryReader::Fail(const char* message)
{
    error_ = message;
    error_ += " at byte offset ";
    error_ += std::to_string(AbsoluteOffset());
    return false;
}

bool EditorBinaryReader::Seek(std::size_t position)
{
    if (position > size_)
        return Fail("seek exceeds reader bounds");
    position_ = position;
    return true;
}

bool EditorBinaryReader::Skip(std::size_t size)
{
    if (size > Remaining())
        return Fail("skip exceeds reader bounds");
    position_ += size;
    return true;
}

bool EditorBinaryReader::ReadBytes(void* destination, std::size_t size)
{
    if (size > Remaining())
        return Fail("read exceeds reader bounds");
    if (size != 0)
        std::memcpy(destination, data_ + position_, size);
    position_ += size;
    return true;
}

bool EditorBinaryReader::ReadU16(std::uint16_t& value)
{
    std::uint8_t bytes[2]{};
    if (!ReadBytes(bytes, sizeof(bytes)))
        return false;
    value = static_cast<std::uint16_t>(bytes[0]) |
        (static_cast<std::uint16_t>(bytes[1]) << 8);
    return true;
}

bool EditorBinaryReader::ReadU32(std::uint32_t& value)
{
    std::uint8_t bytes[4]{};
    if (!ReadBytes(bytes, sizeof(bytes)))
        return false;
    value = static_cast<std::uint32_t>(bytes[0]) |
        (static_cast<std::uint32_t>(bytes[1]) << 8) |
        (static_cast<std::uint32_t>(bytes[2]) << 16) |
        (static_cast<std::uint32_t>(bytes[3]) << 24);
    return true;
}

bool EditorBinaryReader::ReadFloat(float& value)
{
    static_assert(sizeof(float) == sizeof(std::uint32_t),
        "historical scene floats require 32-bit IEEE storage");
    std::uint32_t bits = 0;
    if (!ReadU32(bits))
        return false;
    std::memcpy(&value, &bits, sizeof(value));
    return true;
}

bool EditorBinaryReader::ReadCString(std::string& value,
    std::size_t maximumLength)
{
    value.clear();
    const std::size_t available = Remaining();
    const std::size_t scanLimit = available < maximumLength + 1
        ? available : maximumLength + 1;
    for (std::size_t index = 0; index < scanLimit; ++index)
    {
        const std::uint8_t character = data_[position_ + index];
        if (character == 0)
        {
            value.assign(reinterpret_cast<const char*>(data_ + position_),
                index);
            position_ += index + 1;
            return true;
        }
    }
    return Fail(available > maximumLength
        ? "zero-terminated string exceeds length limit"
        : "zero-terminated string is truncated");
}

bool EditorBinaryReader::Slice(std::size_t size, EditorBinaryReader& result)
{
    if (size > Remaining())
        return Fail("slice exceeds reader bounds");
    result = EditorBinaryReader(data_ + position_, size, AbsoluteOffset());
    position_ += size;
    return true;
}
