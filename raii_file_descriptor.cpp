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
