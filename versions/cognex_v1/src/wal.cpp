#include "wal.h"
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <stdexcept>

void append_and_fsync(const std::string& path, const std::string& record)
{
	int fd = open(path.c_str(),O_CREAT | O_WRONLY | O_APPEND, 0644);

	if(fd<0) throw std::runtime_error("open WAL failed");

	ssize_t n = write(fd,record.data(),record.size());

	if(n != (ssize_t) record.size())
	{
		close(fd);
		throw std::runtime_error("write WAL failed");
	}

	if(fsync(fd) != 0)
	{
		close(fd);
		throw std::runtime_error("fsync failed");
	}

	close(fd);
}
