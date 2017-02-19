#ifndef PROXY_SERVER_EPOLL_WRAPPER_H
#define PROXY_SERVER_EPOLL_WRAPPER_H

#include "raii_file_descriptor.h"
#include "throw_error.h"

#include <iostream>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <set>
#include <functional>
#include <vector>

const int MAX_EVENTS = 100;
typedef std::function<void(uint32_t)> action_t;

class data_info;

class epoll {
    friend class data_info;
private:
    raii_file_descriptor fd;
    bool is_close;
    std::set<data_info*> open_data;

    void ctl_common(raii_file_descriptor& fd, int operation, uint32_t flags, data_info* data);
    void add(raii_file_descriptor& fd_, uint32_t flags, data_info* data);
    void remove(raii_file_descriptor& fd_, data_info* data);
    void modify(raii_file_descriptor& fd_, uint32_t flags, data_info* data);
public:
    epoll();

    raii_file_descriptor create_signals(std::vector<uint8_t> signals);
    void run();
};

class data_info {
    friend class epoll;
private:
    uint32_t flags;
    raii_file_descriptor& fd;
    epoll& ep;
    action_t callback;
public:
    data_info(epoll& ep, raii_file_descriptor& fd, uint32_t flags, action_t action);
    ~data_info();

    void add_flag(uint32_t flag);
    void remove_flag(uint32_t flag);
};

#endif //PROXY_SERVER_EPOLL_WRAPPER_H
