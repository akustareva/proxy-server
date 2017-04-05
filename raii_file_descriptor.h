#ifndef PROXY_SERVER_RAII_FILE_DESCRIPTOR_H
#define PROXY_SERVER_RAII_FILE_DESCRIPTOR_H

#include <unistd.h>
#include <iostream>
#include <sys/ioctl.h>

#include "throw_error.h"

class raii_file_descriptor {
private:
    int fd;
public:
    raii_file_descriptor();
    raii_file_descriptor(int fd);
    ~raii_file_descriptor();

    int get_file_descriptor();
    ssize_t read(void *buffer, size_t size);
    ssize_t write(void const *buffer, size_t size);
    int get_available();

    friend std::ostream& operator<<(std::ostream& os, const raii_file_descriptor& fd);
};

inline std::ostream &operator<<(std::ostream &os, const raii_file_descriptor &fd) {
    os << "file descriptor "  << fd.fd;
    return os;
}

#endif //PROXY_SERVER_RAII_FILE_DESCRIPTOR_H
