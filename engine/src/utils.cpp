#include "utils.h"
#include <unistd.h>
#include <stdexcept>
#include <zlib.h>   // for crc32

void write_all(int fd, const void* buf, size_t len)
{
    const char* p = static_cast<const char*>(buf);

    while (len > 0)
    {
        ssize_t n = write(fd, p, len);
        if (n <= 0)
            throw std::runtime_error("write failed");

        p += n;
        len -= n;
    }
}

ssize_t read_all(int fd, void* buf, size_t len)
{
    char* p = static_cast<char*>(buf);
    size_t total = 0;

    while (total < len)
    {
        ssize_t n = read(fd, p + total, len - total);
        if (n <= 0)
            return n;

        total += n;
    }

    return total;
}

ssize_t pread_all(int fd, void* buf, size_t len, off_t offset)
{
    char* p = static_cast<char*>(buf);
    size_t total = 0;

    while (total < len)
    {
        ssize_t n = ::pread(fd, p + total, len - total, offset + total);

        if (n < 0)
            throw std::runtime_error("pread failed");

        if (n == 0)
            throw std::runtime_error("unexpected EOF");

        total += n;
    }

    return total;
}

uint32_t crc32_str(const std::string& s)
{
    return ::crc32(0,
                   reinterpret_cast<const Bytef*>(s.data()),
                   s.size());
}

uint32_t crc32_buf(const void* data, size_t len)
{
    return ::crc32(0,
                   reinterpret_cast<const Bytef*>(data),
                   len);
}

uint32_t crc32_extend(uint32_t prev_crc, const void* data, size_t len)
{
    return ::crc32(prev_crc,
                   reinterpret_cast<const Bytef*>(data),
                   len);
}

std::string clean_token(const std::string_view& token) 
{
    // Remove leading whitespace
    size_t start = 0;
    while (start < token.size() && std::isspace(static_cast<unsigned char>(token[start])))
        ++start;

    // Remove trailing whitespace
    size_t end = token.size();
    while (end > start && std::isspace(static_cast<unsigned char>(token[end - 1])))
        --end;

    return std::string(token.substr(start, end - start));;
}