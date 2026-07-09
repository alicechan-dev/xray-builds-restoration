#ifndef XRAY_VCL_COMPAT_ANSI_STRING_H
#define XRAY_VCL_COMPAT_ANSI_STRING_H

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

class AnsiString
{
public:
    AnsiString() {}

    AnsiString(const char* value)
        : value_(value ? value : "")
    {
    }

    AnsiString(const std::string& value)
        : value_(value)
    {
    }

    AnsiString(int value)
        : value_(std::to_string(value))
    {
    }

    AnsiString(unsigned int value)
        : value_(std::to_string(value))
    {
    }

    AnsiString& operator=(const char* value)
    {
        value_ = value ? value : "";
        return *this;
    }

    AnsiString& operator=(const std::string& value)
    {
        value_ = value;
        return *this;
    }

    const char* c_str() const
    {
        return value_.c_str();
    }

    operator const char*() const
    {
        return c_str();
    }

    int Length() const
    {
        return static_cast<int>(value_.size());
    }

    bool IsEmpty() const
    {
        return value_.empty();
    }

    char& operator[](int one_based_index)
    {
        return value_.at(checked_index(one_based_index));
    }

    const char& operator[](int one_based_index) const
    {
        return value_.at(checked_index(one_based_index));
    }

    AnsiString LowerCase() const
    {
        AnsiString result(*this);
        for (std::string::iterator it = result.value_.begin(); it != result.value_.end(); ++it)
            *it = static_cast<char>(std::tolower(static_cast<unsigned char>(*it)));
        return result;
    }

    AnsiString UpperCase() const
    {
        AnsiString result(*this);
        for (std::string::iterator it = result.value_.begin(); it != result.value_.end(); ++it)
            *it = static_cast<char>(std::toupper(static_cast<unsigned char>(*it)));
        return result;
    }

    AnsiString& sprintf(const char* format, ...)
    {
        if (!format) {
            value_.clear();
            return *this;
        }

        va_list arguments;
        va_start(arguments, format);

        va_list count_arguments;
        va_copy(count_arguments, arguments);
        const int length = std::vsnprintf(0, 0, format, count_arguments);
        va_end(count_arguments);

        if (length < 0) {
            va_end(arguments);
            value_.clear();
            return *this;
        }

        std::vector<char> buffer(static_cast<std::size_t>(length) + 1);
        std::vsnprintf(&buffer[0], buffer.size(), format, arguments);
        va_end(arguments);

        value_.assign(&buffer[0], static_cast<std::size_t>(length));
        return *this;
    }

    AnsiString& operator+=(const AnsiString& other)
    {
        value_ += other.value_;
        return *this;
    }

    friend AnsiString operator+(const AnsiString& left, const AnsiString& right)
    {
        AnsiString result(left);
        result += right;
        return result;
    }

    friend AnsiString operator+(const AnsiString& left, int right)
    {
        return left + AnsiString(right);
    }

    friend AnsiString operator+(const AnsiString& left, unsigned int right)
    {
        return left + AnsiString(right);
    }

    friend bool operator==(const AnsiString& left, const AnsiString& right)
    {
        return left.value_ == right.value_;
    }

    friend bool operator==(const AnsiString& left, const char* right)
    {
        return left.value_ == (right ? right : "");
    }

    friend bool operator==(const char* left, const AnsiString& right)
    {
        return right == left;
    }

    friend bool operator!=(const AnsiString& left, const AnsiString& right)
    {
        return !(left == right);
    }

    friend bool operator!=(const AnsiString& left, const char* right)
    {
        return !(left == right);
    }

    friend bool operator!=(const char* left, const AnsiString& right)
    {
        return !(left == right);
    }

    friend bool operator<(const AnsiString& left, const AnsiString& right)
    {
        return left.value_ < right.value_;
    }

private:
    std::size_t checked_index(int one_based_index) const
    {
        if (one_based_index <= 0)
            throw std::out_of_range("AnsiString uses one-based indexing");
        return static_cast<std::size_t>(one_based_index - 1);
    }

private:
    std::string value_;
};

#endif
