#ifndef PROXY_EPOLL_WRAPPER_H
#define PROXY_EPOLL_WRAPPER_H

#include <iostream>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <functional>
#include <set>
#include <vector>

#include "throw_error.h"
#include "sockets.h"

const int MAX_EVENTS_COUNT = 100;
typedef std::function<void(uint32_t)> action_t;

class data_info;
class epoll {
    friend class data_info;
private:
    int epoll_fd;
    bool is_close;
    std::set<data_info *> open_data;

    void ctl_common(int operation, int fd, data_info* event, uint32_t flags);
    void add(raii_file_descriptor& fd, data_info* event, uint32_t flags);
    void remove(raii_file_descriptor& fd, data_info* event, uint32_t flags);
    void modify(raii_file_descriptor& fd, data_info* event, uint32_t flags);

    raii_file_descriptor create_signal_fd(std::vector<uint8_t> signals);
public:
    epoll();
    ~epoll();

    void run();
};

class data_info {
    friend class epoll;
private:
    uint32_t flags;
    raii_file_descriptor &fd;
    epoll &ep;
    action_t callback;
public:
    data_info(epoll &ep, raii_file_descriptor &fd, uint32_t flags, action_t callback);
    ~data_info();

    void add_flag(uint32_t flag);
    void remove_flag(uint32_t flag);
};


#endif //EPOLL_WRAPPER_H
