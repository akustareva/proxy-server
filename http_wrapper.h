#ifndef PROXY_SERVER_HTTP_WRAPPER_H
#define PROXY_SERVER_HTTP_WRAPPER_H

#include <string>
#include <unordered_map>
#include <algorithm>

#include "throw_error.h"

enum state_t {
       BAD, START, FIRST_LINE, HEADERS, PARTIAL_BODY, FULL_BODY
};

class http_wrapper {
private:
    size_t body_start = 0;

    std::string get_header_value(std::string header);
protected:
    std::string message;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    state_t state = START;

    void update_state();
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

class http_request : public http_wrapper {
private:
    std::string method;
    std::string URI;
    std::string http_version;
    std::string host = "";

    void parse_first_line() override;
public:
    http_request(std::string message);

    std::string get_URI();
    std::string get_host();
    std::string get_request_message();
};

class http_response: public http_wrapper {
private:
    void parse_first_line() override;
public:
    http_response(std::string message);
};

#endif //PROXY_SERVER_HTTP_WRAPPER_H
