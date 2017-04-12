#include "raii_file_descriptor.h"

raii_file_descriptor::raii_file_descriptor() : fd(-1) {}

raii_file_descriptor::raii_file_descriptor(int fd) : fd(fd) {}

raii_file_descriptor::~raii_file_descriptor() {
    if (fd == -1) {
        return;
    }
    int tmp = close(fd);
    if (tmp == -1 && errno != EAGAIN) {
        throw_error("raii_file_descriptor::close()");
    }
    fd = -1;
}

int &raii_file_descriptor::get_file_descriptor() {
    return fd;
}

int raii_file_descriptor::get_available() {
    int available;
    if (ioctl(fd, FIONREAD, &available) == -1) {
        throw_error("Error during getting available bytes");
    }
    return available;
}

int raii_file_descriptor::get_flags() {
    int result = fcntl(fd, F_GETFD);
    if (result == -1) {
        throw_error("Error in get_flags()");
    }
    return result;
}

void raii_file_descriptor::set_flags(uint32_t nex_flags) {
    if (fcntl(fd, F_GETFD, nex_flags) == -1) {
        throw_error("Error in set_flags()");
    }
}

ssize_t raii_file_descriptor::read(void *buffer, size_t size) {
    ssize_t res = ::read(fd, buffer, size);
    if (res == -1) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            throw_error("Error during reading from socket " + fd);
        }
    }
    return res;
}

ssize_t raii_file_descriptor::write(void const *buffer, size_t size) {
    ssize_t res = ::write(fd, buffer, size);
    if (res == -1) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            throw_error("Error during writing into socket " + fd);
        }
    }
    return res;
}

















