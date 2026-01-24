#include "snapshot.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdexcept>
#include <cstring>

void write_snapshot_atomic(
	const std::string& path,
	const std::unordered_map<std::string,std::string>& store
	)
{
	std::string tmp = path + ".tmp";
	// 1. Open temp snapshot
	int fd = open(tmp.c_str(),O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if(fd<0)
	{
		throw std::runtime_error("failed to open snapshot.tmp");
	}

	// 2. Write snapshot contents
	for(const auto& [k,v]: store)
	{
		std::string line = k + "=" + v + "\n";
		if(write(fd,line.data(),line.size())!= (ssize_t)line.size())
		{
			close(fd);
			throw std::runtime_error("failed to write snapshot");
		}
	}

	// 3. Flush snapshot
	if(fsync(fd)!=0)
	{
		close(fd);
		throw std::runtime_error("failed to fsync snapshot");
	}

	close(fd);

	//4. Atomic Rename
	if(rename(tmp.c_str(),path.c_str())!=0)
	{
		throw std::runtime_error("failed to rename snapshot");
	}

	// 5. Fsync dir
	int dirfd = open(".",O_DIRECTORY);
	if (dirfd >= 0) {
    if (fsync(dirfd) != 0) {
        close(dirfd);
        throw std::runtime_error("failed to fsync directory");
    }
    close(dirfd);
}

}

void load_snapshot(
	const std::string& path,
	 std::unordered_map<std::string,std::string>& store
	)
{

	int fd = open(path.c_str(),O_RDONLY);
	if(fd<0)return;//no snapshot yet

	char buf[4096];
	std::string content;

	ssize_t n ;
	while((n = read(fd,buf,sizeof(buf)))>0)
	{
		content.append(buf,n);
	}

	close(fd);

	size_t pos=0;
	while(true)
	{
		size_t end = content.find('\n', pos);
		if(end == std::string::npos)break;

		std::string line = content.substr(pos,end-pos);
		pos = end+1;

		auto eq = line.find('=');
		if(eq == std::string::npos)continue;
		store[(line.substr(0,eq))] = line.substr(eq+1);
	}
}