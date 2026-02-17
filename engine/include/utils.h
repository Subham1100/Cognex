#pragma once

#include <string>
#include <cstddef>
#include <cstdint>

ssize_t read_all(int fd, void* buf, size_t len);
void write_all(int fd, const void* buf, size_t len);
uint32_t crc32_str(const std::string& s);
std::string clean_token(const std::string_view& token);

