#include "epoll_wrapper.h"

epoll::epoll() {
    int epoll_fd = epoll_create(MAX_EVENTS);
    if (epoll_fd == -1) {
        throw_error("Error in epoll()");
    }
    fd = raii_file_descriptor(epoll_fd);
}

raii_file_descriptor epoll::create_signals(std::vector<uint8_t> signals) {
    sigset_t mask;
    sigemptyset(&mask);
    for (int i = 0; i < signals.size(); i++) {
        sigaddset(&mask, signals[i]);
    }
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        throw_error("Error in create_signals() during creating sigprocmask");
    }
    int signal_fd = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
    if (signal_fd == -1) {
        throw_error("Error in create_signals() during creating signal_fd");
    }
    return raii_file_descriptor(signal_fd);
}

void epoll::run() {
    raii_file_descriptor signal_fd = create_signals({SIGINT, SIGTERM});
    data_info ignored(*this, signal_fd, EPOLLIN, [this](uint32_t signal) {     // add signal event
        std::cerr << "Signal " << signal << " was cought" << std::endl;
        this->is_close = true;
    });
    epoll_event events[MAX_EVENTS];
    while (!is_close) {
        int count = epoll_wait(fd.get_file_descriptor(), events, MAX_EVENTS, -1);
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                throw_error("Error in run() during epoll_wait()");
            }
        }
        for (int i = 0; i < count; i++) {
            auto& event = events[i];
            data_info* data = static_cast<data_info*>(event.data.ptr);
            if (open_data.find(data) != open_data.end()) {
                data->callback(event.events);
            }
        }
    }
}

void epoll::ctl_common(raii_file_descriptor &fd_, int operation, uint32_t flags, data_info *data) {
    epoll_event event;
    event.events = flags;
    event.data.ptr = data;
    int res = epoll_ctl(fd.get_file_descriptor(), operation, fd_.get_file_descriptor(), &event);
    if (res < 0) {
        throw_error("Error in ctl_common(): " + operation);
    }
}

void epoll::add(raii_file_descriptor& fd_, uint32_t flags, data_info* data) {
    open_data.insert(data);
    ctl_common(fd_, EPOLL_CTL_ADD, flags, data);
}

void epoll::remove(raii_file_descriptor& fd_, data_info* data) {
    open_data.erase(data);
    ctl_common(fd_, EPOLL_CTL_DEL, 0, data);
}

void epoll::modify(raii_file_descriptor& fd_, uint32_t flags, data_info *data) {
    ctl_common(fd_, EPOLL_CTL_MOD, flags, data);
}

data_info::data_info(epoll& ep, raii_file_descriptor& fd, uint32_t flags, action_t action): ep(ep),
                                                                                            fd(fd),
                                                                                            flags(flags | EPOLLERR | EPOLLRDHUP | EPOLLHUP),
                                                                                            callback(action)

{
    ep.add(fd, flags, this);
}

data_info::~data_info() {
    ep.remove(fd, this);
}

void data_info::add_flag(uint32_t flag) {
    flags |= flag;
    ep.modify(fd, flags, this);
}

void data_info::remove_flag(uint32_t flag) {
    flags &= ~flag;
    ep.modify(fd, flags, this);
}
