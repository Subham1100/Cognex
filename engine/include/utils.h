#pragma once

#include <string>
#include <cstddef>
#include <cstdint>

ssize_t read_all(int fd, void* buf, size_t len);
void write_all(int fd, const void* buf, size_t len);
uint32_t crc32_str(const std::string& s);
uint32_t crc32_buf(const void* data, size_t len);
uint32_t crc32_extend(uint32_t prev_crc, const void* data, size_t len);
ssize_t pread_all(int fd, void* buf, size_t len, off_t offset);
std::string clean_token(const std::string_view& token);

