#include "epoll_wrapper.h"

epoll::epoll() : is_close(false) {
    epoll_fd = epoll_create(1);
    if (epoll_fd == -1) {
        throw_error("Error in epoll_create");
    }
}

epoll::~epoll() {
    close(epoll_fd);
}

void epoll::run() {
    raii_file_descriptor signal_fd = create_signal_fd({SIGINT, SIGTERM});
    data_info signal_event_creation(*this, signal_fd, EPOLLIN, [this](uint32_t) {
        std::cerr << "Signal caught: Success\n";
        this->is_close = true;
    });
    epoll_event events[MAX_EVENTS_COUNT];
    while (!is_close) {
        int count;
        count = epoll_wait(epoll_fd, events, MAX_EVENTS_COUNT, -1);
        if (count < 0) {
            if (errno != EINTR) {
                throw_error("Error in epoll_wait");
            } else {
                break;
            }
        }
        for (int i = 0; i < count; i++) {
            auto &ev = events[i];
            try {
                data_info *x = static_cast<data_info *>(ev.data.ptr);
                if (open_data.find(x) != open_data.end()) {
                    x->callback(ev.events);
                }
            } catch (std::exception &ignored) {}
        }
    }
}

void epoll::ctl_common(int operation, int fd, data_info *event, uint32_t flags) {
    struct epoll_event e_event;
    e_event.data.ptr = event;
    e_event.events = flags;
    if (epoll_ctl(epoll_fd, operation, fd, &e_event) == -1) {
        throw_error("Error in ctl_common(): " + operation);
    }
}

void epoll::add(raii_file_descriptor &fd, data_info *event, uint32_t flags) {
    open_data.insert(event);
    ctl_common(EPOLL_CTL_ADD, fd.get_file_descriptor(), event, flags);
}

void epoll::remove(raii_file_descriptor & fd, data_info *event, uint32_t flags) {
    open_data.erase(event);
    ctl_common(EPOLL_CTL_DEL, fd.get_file_descriptor(), event, 0);
}

void epoll::modify(raii_file_descriptor & fd, data_info *event, uint32_t flags) {
    ctl_common(EPOLL_CTL_MOD, fd.get_file_descriptor(), event, flags);
}

raii_file_descriptor epoll::create_signal_fd(std::vector<uint8_t> signals) {
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

data_info::data_info(epoll &ep, raii_file_descriptor &fd, uint32_t flags, action_t callback)
        : ep(ep),
          fd(fd),
          callback(callback),
          flags(flags | EPOLLERR | EPOLLRDHUP | EPOLLHUP)
{
    ep.add(fd, this, flags);
}

void data_info::add_flag(uint32_t flag) {
    flags |= flag;
    ep.modify(fd, this, flags);
}

void data_info::remove_flag(uint32_t flag) {
    flags &= ~flag;
    ep.modify(fd, this, flags);
}

data_info::~data_info() {
    ep.remove(fd, this, 0);
}
















