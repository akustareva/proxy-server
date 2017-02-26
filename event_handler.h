#ifndef PROXY_SERVER_EVENT_WRAPPER_H
#define PROXY_SERVER_EVENT_WRAPPER_H

#include "epoll_wrapper.h"

#include <sys/eventfd.h>

typedef std::function<void()> on_event_t;

class event_handler {
private:
    raii_file_descriptor fd;
    on_event_t callback;
    data_info data;

    int create_eventfd();
public:
    event_handler(epoll& ep, on_event_t callback);

    raii_file_descriptor& get_file_descriptor();
};

#endif //PROXY_SERVER_EVENT_WRAPPER_H
