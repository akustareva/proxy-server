#ifndef PROXY_SERVER_HTTP_WRAPPER_H
#define PROXY_SERVER_HTTP_WRAPPER_H

#include <string>
#include <unordered_map>
#include <algorithm>

enum state_t {
       BAD, START, FIRST_LINE, HEADERS, PARTIAL_BODY, FULL_BODY
};

class http_wrapper {
private:
    std::string message;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    state_t state = START;
    size_t body_start = 0;

    void update_state();
    std::string get_header_value(std::string header);
protected:
    virtual void parse_first_line() = 0;
    void parse_headers();
    void check_body();
public:
    http_wrapper(std::string input) : message(input) {};
    ~http_wrapper() {};

    int get_state();
    void add_part(std::string new_part);
    std::string get_message();
};

#endif //PROXY_SERVER_HTTP_WRAPPER_H
