
#ifndef TINYNETFLOW_LOGGER_H
#define TINYNETFLOW_LOGGER_H
/*!
 * \brief 日志库spdlog 的封装
 * thanks： https://github.com/gqw/spdlog_wrapper/blob/main/include/wlog.hpp */

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/logger.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/bundled/printf.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <filesystem>
#include <sstream>
#include <memory>
#include <atomic>


//#include <iostream>

namespace netflow::base {

class Logger final {
public:
    /// 模拟流式输出 stream
    struct log_stream : public std::ostringstream
    {
    public:
        log_stream(const spdlog::source_loc& _loc, spdlog::level::level_enum _lvl, std::string_view _prefix)
                : loc(_loc),
                  lvl(_lvl),
                  prefix(_prefix)
        {
        }

        ~log_stream()
        {
            flush();
        }

        void flush()
        {
            Logger::get().log(loc, lvl, (prefix + str()).c_str());
        }

    private:
        spdlog::source_loc loc;
        spdlog::level::level_enum lvl = spdlog::level::info;
        std::string prefix;
    };

public:
    static Logger& get() {
        static Logger logger;
        return logger;
    }

    bool init(std::string_view log_file_path);
    static const char* get_shortname(std::string_view path);

    void shutdown() { spdlog::shutdown(); }

    template <typename... Args>
    void log(const spdlog::source_loc& loc, spdlog::level::level_enum lvl, const char* fmt, const Args &... args)
    {
        spdlog::log(loc, lvl, fmt, args...);
    }

    template <typename... Args>
    void printf(const spdlog::source_loc& loc, spdlog::level::level_enum lvl, const char* fmt, const Args &... args)
    {
        spdlog::log(loc, lvl, fmt::sprintf(fmt, args...).c_str());
    }

    spdlog::level::level_enum level() {
        return _log_level;
    }

    void set_level(spdlog::level::level_enum lvl) {
        _log_level = lvl;
        spdlog::set_level(lvl);

    }

    void set_flush_on(spdlog::level::level_enum lvl) {
        spdlog::flush_on(lvl);
    }

    void set_fatal_handle() {
        spdlog::set_error_handler([](const std::string& msg){
            // std::cerr << "my err handler: " << msg << std::endl;
            std::abort();
        });
    }


private:
    Logger() = default;
    ~Logger() = default;

    Logger(const Logger&) = delete;
    void operator=(const Logger&) = delete;

private:
    std::atomic_bool _is_inited = false;
    spdlog::level::level_enum _log_level = spdlog::level::trace;
};

class logger_none {
public:
    logger_none() = default;

    static logger_none& get() {
        static logger_none logger;
        return logger;
    }

    logger_none& operator<<(const char* content) {
        return *this;
    }
};

}  // namespace netflow::base

#endif //TINYNETFLOW_LOGGER_H
