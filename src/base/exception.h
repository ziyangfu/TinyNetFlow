//
// Created by fzy on 23-3-22.
//

//! 异常
#ifndef LIBZV_EXCEPTION_H
#define LIBZV_EXCEPTION_H

#include "types.h"
#include <exception>
namespace muduo {

class Exception : public std::exception {
public:
    Exception(string what);
    ~Exception() noexcept override = default;
    const char* what() const noexcept override {
            return message_.c_str();
    }
    const char* stackTrace() const noexcept {
        return stack_.c_str();
    }
private:
    string message_;
    string stack_;

};

} // muduo

#endif //LIBZV_EXCEPTION_H
