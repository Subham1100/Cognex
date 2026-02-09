#include "wal.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstdint>
#include <string>
#include <string_view>
#include <zlib.h>

// ---------- helpers ----------

static void write_all(int fd, const void* buf, size_t len)
{
	const char* p = static_cast<const char*>(buf);
	while(len > 0 )
	{
		ssize_t n  = write(fd, p, len);
		if(n<=0) throw std::runtime_error("write failed");
		p+=n;
		len -= n;
	}
}

ssize_t read_all(int fd, void* buf, size_t len) {
     char* p = static_cast<char*>(buf);
    size_t total = 0;

    while (total < len) {
        ssize_t n = read(fd, p + total, len - total);
        if (n <= 0) return n;
        total += n;
    }
    return total;
}



static uint32_t crc32_str(const std::string& s)
{
	return ::crc32(0,reinterpret_cast<const Bytef*>(s.data()),s.size());
}


void append_and_fsync(const WalPath& path, const std::string& record)
{
    int fd = open(path.value.c_str(),O_CREAT | O_WRONLY | O_APPEND, 0644);
    if(fd<0)throw std::runtime_error("open WAL failed");

    uint32_t len = record.size();
    uint32_t checksum = crc32_str(record);

    write_all(fd,&len,sizeof(len));
    write_all(fd,record.data(),record.size());
    write_all(fd,&checksum,sizeof(checksum));

    if(fsync(fd)!=0)
    {
    	close(fd);
    	throw std::runtime_error("fsync WAL error");
    }
    close(fd);
}

// ---------- WAL replay ----------

bool wal_replay(const WalPath& path, void(*apply)(const std::string_view& record))
{
	int fd = open(path.value.c_str(),O_RDONLY);
	if(fd<0)return true;

	while(true)
	{
		uint32_t len;
		ssize_t n = read_all(fd, &len, sizeof(len));
		if(n==0)break;
		if(n!= sizeof(len))break;

		std::string record(len, '\0');
		if(read_all(fd,record.data(),len)!=(ssize_t)len)break;

		uint32_t checksum;
		if(read_all(fd,&checksum,sizeof(checksum))!=sizeof(checksum))break;

		if(crc32_str(record)!= checksum)break;

		apply(record);
	}
	close(fd);
	return true;

}


// ---------- WAL truncate ----------

void wal_truncate(const WalPath& path) {
    int fd = open(path.value.c_str(), O_WRONLY | O_TRUNC);
    if (fd >= 0) {
        fsync(fd);
        close(fd);
    }
}