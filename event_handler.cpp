#include "event_handler.h"

event_handler::event_handler(epoll &ep, std::function<void()> callback)
        : event_fd(create_eventfd()),
          callback(std::move(callback)),
          handler_event(ep, event_fd, EPOLLIN, [this] (uint32_t) {
              uint64_t tmp;
              while (event_fd.read(&tmp, sizeof(tmp)) != -1) {
                  this->callback();
              }
          })
{}

int event_handler::create_eventfd() {
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC | EFD_SEMAPHORE);
    if (fd == -1) {
        throw_error("Error during creation eventfd");
    }
    return fd;
}

raii_file_descriptor &event_handler::get_file_descriptor() {
    return event_fd;
}

