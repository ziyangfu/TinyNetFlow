//
// Created by fzy on 23-5-30.
//

#ifndef TINYNETFLOW_LOGGING_H
#define TINYNETFLOW_LOGGING_H

#include "Logger.h"

namespace netflow::base {

#define LOG_LEVEL_TRACE    0
#define LOG_LEVEL_DEBUG    1
#define LOG_LEVEL_INFO     2
#define LOG_LEVEL_WARN     3
#define LOG_LEVEL_ERROR    4
#define LOG_LEVEL_FATAL    5
#define LOG_LEVEL_CLOSE    6

/*!
 * \brief
 * Logging: 统一日志接口， 底层可以基于spdlog与glog实现
 * 底层基于spdlog：
 *      1. LOG: spdlog 标准输出，C++20输出样式
 *      2. PRINT: C风格输出样式
 *      3. STREAM: C++流式输出风格
 *
 * 底层基于glog：
 *      1. STREAM: C++流式输出风格 **/
// got short filename(exlude file directory)
#define GET_FILENAME (netflow::base::Logger::get_shortname(__FILE__))

#if (LOGGER_LEVEL <= LOG_LEVEL_TRACE)
#	define	 LOG_TRACE(fmt, ...) 		spdlog::log({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::trace, fmt, ##__VA_ARGS__);
#	define 	 PRINT_TRACE(fmt,...) 		netflow::base::Logger::get().printf({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::trace, fmt, ##__VA_ARGS__);
#	define	 STREAM_TRACE 			netflow::base::Logger::log_stream({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::trace, "")
#else
    #	define	 LOG_TRACE(fmt, ...)
#	define 	 PRINT_TRACE(fmt,...)
#	define	 STREAM_TRACE netflow::base::logger_none::get()
#endif

#if (LOGGER_LEVEL <= LOG_LEVEL_DEBUG)
#	define	 LOG_DEBUG(fmt, ...) 		spdlog::log({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::debug, fmt, ##__VA_ARGS__);
#	define 	 PRINT_DEBUG(fmt,...) 		netflow::base::Logger::get().printf({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::debug, fmt, ##__VA_ARGS__);
#	define	 STREAM_DEBUG 			netflow::base::Logger::log_stream({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::debug, "")
#else
    #	define	 LOG_DEBUG(fmt, ...)
#	define 	 PRINT_DEBUG(fmt,...)
#	define	 STREAM_DEBUG netflow::base::logger_none::get()
#endif

#if (LOGGER_LEVEL <= LOG_LEVEL_INFO)
#	define	 LOG_INFO(fmt, ...) 		spdlog::log({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::info, fmt, ##__VA_ARGS__);
#	define 	 PRINT_INFO(fmt,...) 		netflow::base::Logger::get().printf({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::info, fmt, ##__VA_ARGS__);
#	define	 STREAM_INFO 			netflow::base::Logger::log_stream({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::info, "")
#else
    #	define	 LOG_INFO(fmt, ...)
#	define 	 PRINT_INFO(fmt,...)
#	define	 STREAM_INFO netflow::base::logger_none::get()
#endif

#if (LOGGER_LEVEL <= LOG_LEVEL_WARN)
#	define	 LOG_WARN(fmt, ...) 		spdlog::log({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::warn, fmt, ##__VA_ARGS__);
#	define 	 PRINT_WARN(fmt,...) 		netflow::base::Logger::get().printf({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::warn, fmt, ##__VA_ARGS__);
#	define	 STREAM_WARN 			netflow::base::Logger::log_stream({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::warn, "")
#else
    #	define	 LOG_WARN(fmt, ...)
#	define 	 PRINT_WARN(fmt,...)
#	define	 STREAM_WARN netflow::base::logger_none::get()
#endif

#if (LOGGER_LEVEL <= LOG_LEVEL_ERROR)
#	define	 LOG_ERROR(fmt, ...) 		spdlog::log({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::err, fmt, ##__VA_ARGS__);
#	define 	 PRINT_ERROR(fmt,...) 		netflow::base::Logger::get().printf({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::err, fmt, ##__VA_ARGS__);
#	define	 STREAM_ERROR 			netflow::base::Logger::log_stream({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::err, "")
#else
    #	define	 LOG_ERROR(fmt, ...)
#	define 	 PRINT_ERROR(fmt,...)
#	define	 STREAM_ERROR netflow::base::logger_none::get()
#endif

#if (LOGGER_LEVEL <= LOG_LEVEL_FATAL)
#	define	 LOG_FATAL(fmt, ...) 		spdlog::log({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::critical, fmt, ##__VA_ARGS__);
#	define 	 PRINT_FATAL(fmt,...) 		netflow::base::Logger::get().printf({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::critical, fmt, ##__VA_ARGS__);
#	define	 STREAM_FATAL 			netflow::base::Logger::log_stream({GET_FILENAME, __LINE__, __FUNCTION__}, spdlog::level::critical, "")
#else
    #	define	 LOG_FATAL(fmt, ...)
#	define 	 PRINT_FATAL(fmt,...)
#	define	 STREAM_FATAL netflow::base::logger_none::get()
#endif

}  // namespace netflow::base


/*** 网上看到的spdlog的封装
 * #pragma once

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

*/

#endif //TINYNETFLOW_LOGGING_H
