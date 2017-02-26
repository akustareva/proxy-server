#include "raii_file_descriptor.h"
#include "throw_error.h"

raii_file_descriptor::raii_file_descriptor(): fd(-1) {}

raii_file_descriptor::raii_file_descriptor(int fd): fd(fd) {}

raii_file_descriptor::~raii_file_descriptor() {
    if (fd == -1) {
        return;
    }
    int tmp = close(fd);
    if (tmp == -1 && errno != EAGAIN) {
        throw_error("file_descriptor::close()");
    }
    fd = -1;
}

int raii_file_descriptor::get_file_descriptor() {
    return fd;
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
