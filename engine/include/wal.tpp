#include <fcntl.h>
#include <unistd.h>
#include "utils.h"


template<typename ApplyFn>
bool wal_replay(const WalPath& path, ApplyFn apply)
{
    int fd = open(path.value.c_str(), O_RDONLY);
    if (fd < 0)
        return true;

    while (true)
    {
        uint32_t len;
        ssize_t n = read_all(fd, &len, sizeof(len));

        if (n == 0) break;              // EOF
        if (n != sizeof(len)) break;    // partial read

        std::string record(len, '\0');
        if (read_all(fd, record.data(), len) != (ssize_t)len)
            break;

        uint32_t checksum;
        if (read_all(fd, &checksum, sizeof(checksum)) != (ssize_t)sizeof(checksum))
            break;

        if (crc32_str(record) != checksum)
            break;

        apply(std::string_view(record));
    }

    close(fd);
    return true;
}
