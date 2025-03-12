
/** 实现程序的加载
 * 1. 从命令行中读取到app文件夹，编译文件夹，逐个启动各个APP
 * 2. 启动reactor循环，根据收到的消息，确定app的当前运行状态
 *
 * 方案2： 使用systemd来管理app，作为执行管理程序
 * */

#include <iostream>
#include "process/Process.h"
#include "argparse/argparse.hpp"
#include "spdlog/spdlog.h"  /** 注意 spdlog与fmt的顺序 */
#if __cplusplus >= 202002L
#include <format>  // 编译没问题，但CLion里找不到,暂时注释掉它
#endif

#include "CmdArgs.h"

int cmdParser(argparse::ArgumentParser& parser, CmdArgs& cmdArgs) {
    parser.add_argument("-c", "--config")
            .help("please give the configure file")
            .default_value("")
            .store_into(cmdArgs.configFile);
    parser.add_argument("-a", "--apps")
            .help("please give the apps directory")
            .default_value("")
            .store_into(cmdArgs.appsDir);
    parser.add_argument("-s", "--start")
            .help("start only one app, please give the full path of app")
            .default_value("")
            .store_into(cmdArgs.startApp);
    parser.add_argument("-v", "--version")           /** 使用自定义的版本显示 */
            .help("Output version information and exit")
            .default_value(false)
            .implicit_value(true)
            .action(
                    [](const std::string& value) {
                        SPDLOG_INFO("execmd version: 0.0.1");
                        exit(0);
                    }
            );
    return 0;
}

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::info);
    argparse::ArgumentParser parser("execmd", "1.0", argparse::default_arguments::help);
    CmdArgs cmdArgs;
    cmdParser(parser, cmdArgs);
    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        SPDLOG_ERROR("{}", err.what());
        return 1;
    }
    //osadaptor::process::Process process;
    //process.processCreate(startApp, {"-a", "-l"});
    //while (1) {}

    return 0;
}