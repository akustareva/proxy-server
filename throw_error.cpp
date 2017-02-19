#include "throw_error.h"

void throw_error(std::string msg) {
    throw std::runtime_error(msg);
}
