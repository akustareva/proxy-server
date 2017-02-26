#include "event_handler.h"

int event_handler::create_eventfd() {
    int event_fd = eventfd(0, EFD_SEMAPHORE | EFD_NONBLOCK | EFD_CLOEXEC);
    if (event_fd == -1) {
        throw_error("Error during creation eventfd");
    }
    return event_fd;
}

event_handler::event_handler(epoll& ep, on_event_t callback):
        fd(create_eventfd()),
        callback(std::move(callback)),
        data(ep, fd, EPOLLIN, [this] (uint32_t) {
            uint64_t tmp;
            while (fd.read(&tmp, sizeof(tmp)) != -1) {
                this->callback();
            }
        })
{}

raii_file_descriptor& event_handler::get_file_descriptor() {
    return fd;
}
