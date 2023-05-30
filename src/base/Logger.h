
#ifndef TINYNETFLOW_LOGGER_H
#define TINYNETFLOW_LOGGER_H
/*!
 * \brief spdlog 的封装
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

namespace netflow::base {

class Logger final {
public:
    /// 模拟流式输出 stream
    struct log_stream : public std::ostringstream
    {
    public:
        log_stream(const spdlog::source_loc& _loc, spdlog::level::level_enum _lvl, std::string_view _prefix)
                : loc(_loc)
                , lvl(_lvl)
                , prefix(_prefix)
        {
        }

        ~log_stream()
        {
            flush();
        }

        void flush()
        {
            logger::get().log(loc, lvl, (prefix + str()).c_str());
        }

    private:
        spdlog::source_loc loc;
        spdlog::level::level_enum lvl = spdlog::level::info;
        std::string prefix;
    };

public:
    static logger& get() {
        static logger logger;
        return logger;
    }

    bool init(std::string_view log_file_path) {
        namespace fs = std::filesystem;
        if (_is_inited) return true;
        try
        {
            // check log path and try to create log directory
            fs::path log_path(log_file_path);
            fs::path log_dir = log_path.parent_path();
            if (!fs::exists(log_path)) {
                fs::create_directories(log_dir);
            }
            // initialize spdlog
            constexpr std::size_t log_buffer_size = 32 * 1024; // 32kb
            // constexpr std::size_t max_file_size = 50 * 1024 * 1024; // 50mb
            spdlog::init_thread_pool(log_buffer_size, std::thread::hardware_concurrency());
            std::vector<spdlog::sink_ptr> sinks;
            auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_path.string(), 0, 2);
            sinks.push_back(daily_sink);

            // auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path.string(), true);
            // sinks.push_back(file_sink);

#if defined(_DEBUG) && defined(WIN32) && !defined(NO_CONSOLE_LOG)
            auto ms_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
			sinks.push_back(ms_sink);
#endif //  _DEBUG

#if !defined(WIN32) && !defined(NO_CONSOLE_LOG)
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            sinks.push_back(console_sink);
#endif
            spdlog::set_default_logger(std::make_shared<spdlog::logger>("", sinks.begin(), sinks.end()));

            spdlog::set_pattern("%s(%#): [%L %D %T.%e %P %t %!] %v");
            spdlog::flush_on(spdlog::level::warn);
            spdlog::set_level(_log_level);
        }
        catch (std::exception_ptr e)
        {
            assert(false);
            return false;
        }
        _is_inited = true;
        return true;
    }

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

    static const char* get_shortname(std::string_view path) {
        if (path.empty())
            return path.data();

        size_t pos = path.find_last_of("/\\");
        return path.data() + ((pos == path.npos) ? 0 : pos + 1);
    }

private:
    logger() = default;
    ~logger() = default;

    logger(const logger&) = delete;
    void operator=(const logger&) = delete;

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



#define LOG_LEVEL_TRACE    0
#define LOG_LEVEL_DEBUG    1
#define LOG_LEVEL_INFO     2
#define LOG_LEVEL_WARN     3
#define LOG_LEVEL_ERROR    4
#define LOG_LEVEL_FATAL    5
#define LOG_LEVEL_CLOSE    6


// got short filename(exlude file directory)
#define __FILENAME__ (wlog::logger::get_shortname(__FILE__))

#if (LOGGER_LEVEL <= LOG_LEVEL_TRACE)
#	define	 LOG_TRACE(fmt, ...) 		spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::trace, fmt, ##__VA_ARGS__);
#	define 	 PRINT_TRACE(fmt,...) 		wlog::logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::trace, fmt, ##__VA_ARGS__);
#	define	 STREAM_TRACE() 			wlog::logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::trace, "")
#else
    #	define	 LOG_TRACE(fmt, ...)
#	define 	 PRINT_TRACE(fmt,...)
#	define	 STREAM_TRACE() wlog::logger_none::get()
#endif

#if (LOGGER_LEVEL <= LOG_LEVEL_DEBUG)
#	define	 LOG_DEBUG(fmt, ...) 		spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::debug, fmt, ##__VA_ARGS__);
#	define 	 PRINT_DEBUG(fmt,...) 		wlog::logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::debug, fmt, ##__VA_ARGS__);
#	define	 STREAM_DEBUG() 			wlog::logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::debug, "")
#else
    #	define	 LOG_DEBUG(fmt, ...)
#	define 	 PRINT_DEBUG(fmt,...)
#	define	 STREAM_DEBUG() wlog::logger_none::get()
#endif

#if (LOGGER_LEVEL <= LOG_LEVEL_INFO)
#	define	 LOG_INFO(fmt, ...) 		spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::info, fmt, ##__VA_ARGS__);
#	define 	 PRINT_INFO(fmt,...) 		wlog::logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::info, fmt, ##__VA_ARGS__);
#	define	 STREAM_INFO() 			wlog::logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::info, "")
#else
    #	define	 LOG_INFO(fmt, ...)
#	define 	 PRINT_INFO(fmt,...)
#	define	 STREAM_INFO() wlog::logger_none::get()
#endif

#if (LOGGER_LEVEL <= LOG_LEVEL_WARN)
#	define	 LOG_WARN(fmt, ...) 		spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::warn, fmt, ##__VA_ARGS__);
#	define 	 PRINT_WARN(fmt,...) 		wlog::logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::warn, fmt, ##__VA_ARGS__);
#	define	 STREAM_WARN() 			wlog::logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::warn, "")
#else
    #	define	 LOG_WARN(fmt, ...)
#	define 	 PRINT_WARN(fmt,...)
#	define	 STREAM_WARN() wlog::logger_none::get()
#endif

#if (LOGGER_LEVEL <= LOG_LEVEL_ERROR)
#	define	 LOG_ERROR(fmt, ...) 		spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::error, fmt, ##__VA_ARGS__);
#	define 	 PRINT_ERROR(fmt,...) 		wlog::logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::error, fmt, ##__VA_ARGS__);
#	define	 STREAM_ERROR() 			wlog::logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::error, "")
#else
    #	define	 LOG_ERROR(fmt, ...)
#	define 	 PRINT_ERROR(fmt,...)
#	define	 STREAM_ERROR() wlog::logger_none::get()
#endif

#if (LOGGER_LEVEL <= LOG_LEVEL_FATAL)
#	define	 LOG_FATAL(fmt, ...) 		spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::fatal, fmt, ##__VA_ARGS__);
#	define 	 PRINT_FATAL(fmt,...) 		wlog::logger::get().printf({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::fatal, fmt, ##__VA_ARGS__);
#	define	 STREAM_FATAL() 			wlog::logger::log_stream({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::fatal, "")
#else
    #	define	 LOG_FATAL(fmt, ...)
#	define 	 PRINT_FATAL(fmt,...)
#	define	 STREAM_FATAL() wlog::logger_none::get()
#endif


}  // namespace netflow::base

#endif //TINYNETFLOW_LOGGER_H
