#include "wal.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstdint>
#include <string>
#include <string_view>
#include <zlib.h>
#include <utils.h>

void append_and_fsync(const WalPath& path, const std::string& record, size_t& walWrites_,size_t& walFsyncEveryNWrites_)
{
    int fd = open(path.value.c_str(),O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (fd < 0)
    {throw std::runtime_error( std::string("open WAL failed at ") + __func__);
    }

    uint32_t len = static_cast<uint32_t>(record.size());
    uint32_t checksum = crc32_str(record);

    write_all(fd,&len,sizeof(len));
    write_all(fd,record.data(),record.size());
    write_all(fd,&checksum,sizeof(checksum));

    
    walWrites_++;

    if(walWrites_ >=walFsyncEveryNWrites_)
    {
        if(fsync(fd)!=0)
        {
            close(fd);
            throw std::runtime_error(std::string("open WAL failed at ") + __func__);
        }
        walWrites_ = 0;
    }
    close(fd);
}

// ---------- WAL replay ----------

// moved to header


// ---------- WAL truncate ----------

void wal_truncate(const WalPath& path) {
    int fd = open(path.value.c_str(), O_WRONLY | O_TRUNC);
    if (fd >= 0) {
        fsync(fd);
        close(fd);
    }
}