//
// Created by fzy on 23-5-30.
//
// 基本想法是创建一个log适配层，防止万一想替换目前再用的spdlog，例如替换为 glog，或者自己写一个log
#ifndef TINYNETFLOW_LOG_LOGADAPTOR_H
#define TINYNETFLOW_LOG_LOGADAPTOR_H

#include <iostream>
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>

// 参考1：https://blog.csdn.net/gongjianbo1992/article/details/113279632
// 参考2：https://techartlife.com/%E8%AE%A1%E7%AE%97%E6%9C%BA/%E7%BC%96%E7%A8%8B/%E7%BC%96%E7%A8%8B%E8%AF%AD%E8%A8%80/cpp/%E5%B0%81%E8%A3%85spdlog%E5%BA%93/
// 参考3：https://github.com/SergiusTheBest/plog/blob/master/include/plog/Log.h

#include "exception.hpp"
#include <memory>
#include <string>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace Log {

struct log_execption : virtual base_exception {};

class SPDLOG {
private:
    SPDLOG() = default;
private:
    std::shared_ptr<spdlog::logger> logger_ptr_;
    void setLogLevel(const std::string& level);

public:
	static SPDLOG& getInstance() {
		static SPDLOG instance;
		return instance;
	}
    // 初始化一个默认日志文件logger: 日志路径；logger name; 日志等级；单个日志文件最大大小；回滚日志文件个数；日志是否线程安全；
	void init(std::string log_file_path,std::string logger_name, std::string level, size_t max_file_size, size_t max_files, bool mt_security = false); 
    std::shared_ptr<spdlog::logger> logger() { return logger_ptr_; }
}; // SPDLOG class

#define LOG_TRACE(...)       SPDLOG::getInstance().logger().get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)       SPDLOG::getInstance().logger().get()->debug(__VA_ARGS__)
#define LOG_INFO(...)        SPDLOG::getInstance().logger().get()->info(__VA_ARGS__)
#define LOG_WARN(...)        SPDLOG::getInstance().logger().get()->warn(__VA_ARGS__)
#define LOG_ERROR(...)       SPDLOG::getInstance().logger().get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)    SPDLOG::getInstance().logger().get()->critical(__VA_ARGS__)
} // Log namespace

void Log::SPDLOG::init(std::string log_file_path, std::string logger_name, std::string level, size_t max_file_size, size_t max_files, bool mt_security)
{
    try {
        if (mt_security) {
            logger_ptr_ = spdlog::rotating_logger_mt(logger_name, log_file_path, max_file_size, max_files);
        }
        else {
            logger_ptr_ = spdlog::rotating_logger_st(logger_name, log_file_path, max_file_size, max_files);
        }
        setLogLevel(level);
        logger_ptr_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s %!:%#] %v"); //设置格式:https://spdlog.docsforge.com/v1.x/3.custom-formatting/
    } 
    catch (const spdlog::spdlog_ex& ex) {
       BOOST_THROW_EXCEPTION(log_execption()
            << err_str("Log initialization failed: " + std::string(ex.what()))
        );
    }
}

void Log::SPDLOG::setLogLevel(const std::string &level)
{
    char L = toupper(level[0]);
    if (L == 'T') { // trace
        logger_ptr_->set_level(spdlog::level::trace);
        logger_ptr_->flush_on(spdlog::level::trace);
    }
    else if (L == 'D') { // debug
        logger_ptr_->set_level(spdlog::level::debug);
        logger_ptr_->flush_on(spdlog::level::debug);
    }
    else if (L == 'I') { // info
        logger_ptr_->set_level(spdlog::level::info);
        logger_ptr_->flush_on(spdlog::level::info);
    }
    else if (L == 'W') { // warn
        logger_ptr_->set_level(spdlog::level::warn);
        logger_ptr_->flush_on(spdlog::level::warn);
    }
    else if (L == 'E') { // error
        logger_ptr_->set_level(spdlog::level::err);
        logger_ptr_->flush_on(spdlog::level::err);
    }
    else if (L == 'C') { // critical
        logger_ptr_->set_level(spdlog::level::critical);
        logger_ptr_->flush_on(spdlog::level::critical);
    } else {
        BOOST_THROW_EXCEPTION(log_execption()
            << err_str("level set error " + level)
        );
    }
}



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
