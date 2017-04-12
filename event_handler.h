#ifndef PROXY_SERVER_EVENT_HANDLER_H
#define PROXY_SERVER_EVENT_HANDLER_H

#include <sys/eventfd.h>

#include "epoll_wrapper.h"

typedef std::function<void()> on_event_t;

class event_handler {
private:
    raii_file_descriptor event_fd;
    on_event_t callback;
    data_info handler_event;

    int create_eventfd();
public:
    event_handler(epoll &ep, on_event_t callback);

    raii_file_descriptor & get_file_descriptor();
};


#endif //PROXY_SERVER_EVENT_HANDLER_H
