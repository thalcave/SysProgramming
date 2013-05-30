/* Write a program like cp that, when used to copy a regular file that contains holes
(sequences of null bytes), also creates corresponding holes in the target file.
*/

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

void
doCopy(std::string const& src, std::string const& dst, bool sparse)
{
	//open src
	int src_fd = ::open(src.c_str(), O_RDONLY);
	if (-1 == src_fd)
	{
		throw std::runtime_error("Error opening file [" + src + "]: " + strerror(errno));
	}
	
	//open dst
	int dst_fd = ::open(dst.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
	if (-1 == dst_fd)
	{
		::close(src_fd);
		throw std::runtime_error("Error opening file [" + dst + "]: " + strerror(errno));
	}
	
	const unsigned BUF_SIZE = 65536;
	char buf[BUF_SIZE];
	
	ssize_t num_read;
	while ((num_read = ::read(src_fd, buf, BUF_SIZE)) > 0)
	{
		//write in dst
		ssize_t num_written = ::write(dst_fd, buf, num_read);
		if (num_written != num_read)
		{
			std::string error_msg;
			if (num_written < 0)
			{ 
				error_msg = "Error writing to dest: " + std::string(strerror(errno));
			}
			else
			{
				error_msg = "Could not write whole buffer";
			}
			
			::close(src_fd);
			::close(dst_fd);
			throw std::runtime_error(error_msg);
		}
		
	}
	
	if (num_read < 0)
	{
		std::string error_msg = "Error reading from src: " + std::string(strerror(errno));
		::close(src_fd);
		::close(dst_fd);
		throw std::runtime_error(error_msg);
	}
	
	::close(src_fd);
	::close(dst_fd);
}

// cp [--sparse] src dest
int main(int argc, char* argv[])
{
	if (argc != 4 && argc != 3)
	{
		std::cerr<<"Usage: ./mcp [--sparse] src dst\n";
		return -1;
	}
	
	bool sparse = false;

	static struct option long_options[] =
	{
		{"sparse", no_argument, NULL, 's'},
		{0, 0, 0, 0}
	};

	// loop over all of the options
	char ch;
	while ((ch = getopt_long(argc, argv, "s:", long_options, NULL)) != -1)
	{
		// check to see if a single character or long option came through
		switch (ch)
		{
			// short option 's'
			case 's':
				//std::cout<<"found optarg: "<<optarg<<"\n";
//				std::cout<<"found sparse: \n";
				sparse = true;
				break;
		}
	}
	
//	std::cout<<"optind: "<<optind<<"\n";
//	std::cout<<"argc : "<<argc<<"\n";
	
	if (optind >= argc)
	{
		std::cerr<<"Usage: ./mcp [--sparse] src dst\n";
		return -1;
	}
	
	std::string src(argv[optind++]);
	std::string dst(argv[optind]);
	
	std::cout<<"src: "<<src <<" dest: "<<dst<<" sparse? "<<(sparse ? std::string("yes"):std::string("no"))<<"\n";
	
	try
	{
		doCopy(src, dst, sparse);
	} catch (std::runtime_error const& rte) {
		std::cerr<<"RuntimeError: "<<rte.what()<<"\n";
		return -1;
	} catch (std::exception const& rte) {
		std::cerr<<"Exception: "<<rte.what()<<"\n";
		return -1;
	}

	return 0;
}
