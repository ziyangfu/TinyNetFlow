//
// Created by fzy on 23-5-30.
//
#include "Logger.h"

#include <assert.h>

using namespace netflow::base;

bool Logger::init(std::string_view log_file_path) {
    namespace fs = std::filesystem;
    if (_is_inited)
        return true;
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


const char* Logger::get_shortname(std::string_view path) {
    if (path.empty())
        return path.data();

    size_t pos = path.find_last_of("/\\");
    return path.data() + ((pos == path.npos) ? 0 : pos + 1);
}