//
// Created by fzy on 23-3-22.
//

#include "exception.h"
#include "current_thread.h"

namespace muduo {
    Exception::Exception(string msg)
    : message_(std::move(msg)),
      stack_(CurrentThread::stackTrace(false))
      {}
} // muduo