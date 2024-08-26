//
// Created by fzy on 23-5-30.
//
// 基本想法是创建一个log适配层，防止万一想替换目前再用的spdlog，例如替换为 glog，或者自己写一个log
#ifndef TINYNETFLOW_LOG_LOGADAPTOR_H
#define TINYNETFLOW_LOG_LOGADAPTOR_H

#include <iostream>
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#define LOG_TRACE std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_DEBUG std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_INFO  std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_WARN  std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_ERROR std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_FATAL std::cout << __FILE__ << ":" << __LINE__ << " "
#define CHECK_NOTnullptr(val) LOG_ERROR << "'" #val "' Must be non nullptr";

#else
// 如果没有使用 spdlog，则定义一个空的日志器
struct Logger {
    template<typename... Args>
    void log(int level, const char* file, int line, Args&&... args) const {
        std::cout << file << ":" << line << " ";
        (std::cout << ... << std::forward<Args>(args)) << '\n';
    }
};
#define LOG_TRACE std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_DEBUG std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_INFO  std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_WARN  std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_ERROR std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_FATAL std::cout << __FILE__ << ":" << __LINE__ << " "
#define CHECK_NOTnullptr(val) LOG_ERROR << "'" #val "' Must be non nullptr";

#endif


#endif // TINYNETFLOW_LOG_LOGADAPTOR_H
