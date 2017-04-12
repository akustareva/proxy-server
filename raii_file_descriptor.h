#ifndef PROXY_SERVER_FILE_DESCRIPTOR_H
#define PROXY_SERVER_FILE_DESCRIPTOR_H

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "throw_error.h"

class raii_file_descriptor {
private:
    int fd;
public:
    raii_file_descriptor();
    raii_file_descriptor(int fd);
    ~raii_file_descriptor();

    int& get_file_descriptor();
    int get_available();
    int get_flags();
    void set_flags(uint32_t nex_flags);
    ssize_t read(void *buffer, size_t size);
    ssize_t write(void const *buffer, size_t size);
};


#endif //PROXY_SERVER_FILE_DESCRIPTOR_H
