/*!
 * \brief
 * 1. 进程创建-程序加载器
 *      1. 程序名称
 *      2. 程序参数
 *      3. 程序执行文件夹
 *      4. 环境变量
 * 2. 进程调度
 *      1. 调度策略与优先级设置
 *      2. CPU 亲和性设置（可选）
 *      2. 时间触发调度器（RT, option）
 * 3. 进程资源管理
 *      1. cgroups 限制进程资源
 *              https://tech.meituan.com/2015/03/31/cgroups.html
 *
 * */

#ifndef OSADAPTOR_PROCESS_PROCESS_H
#define OSADAPTOR_PROCESS_PROCESS_H

#include <string>
#include <vector>
#include "process/Types.h"
#include "process/ProcessSetting.h"

namespace osadaptor::process {

/** 如何描述一个进程？ 每一个进程存在一个Process实例， 并在生命周期内常驻内存 ？
 * 当收到重启信号时，直接重启
 * 命令参数/程序设定/工作目录 可以修改更新，程序名与程序路径不允许修改
 *
 * 一个二进制程序文件，可以有多个进程实例
 * Process 唯一描述一个进程示例，而不是一个二进制程序文件，创建的多个实例
 * 多个实例由 ProcessCluster描述
 * */
class Process {
public:
    Process(std::string programPath, std::vector<std::string> args,
            ProcessSettings settings);
    ~Process();
    void processCreate();
    std::string& getProgramName();
    std::string& getCurrentWorkDir();
    void updateProcessSettings(const ProcessSettings& newSettings);
    void updateArgs(const std::vector<std::string>& newArgs);
    void updateCurrentWorkDir(const std::string& newDir);
    ProcessSettings& getCurrentProcessSettings();
    std::string getCurrentSchedulePolicyStr();
    pid_t getCurrentProcessPid() const;

    void sendSIGKILL();
    void sendSIGTERM();
    void waitPid() { /* empty */}
    void processSync() { /* empty */}
private:
    bool checkExecutable();
    void setCpuAffinity();
    bool configureScheduler();
    static int getSystemCpuCoreCount();
    static bool isValidCpuSet(const cpu_set_t& cpuSet, int numCpus);
    static bool isSuperuserPrivileges();
    static std::string schedulerPolicyToString(SchedulerPolicy policy);
private:
    pid_t processPid_;                  /** child process pid    */
    std::string programPath_;           /** eg： /usr/bin/ls     */
    std::string programName_;           /** eg: ls               */
    std::vector<std::string> args_;     /** eg： "-a", "-l"       */
    std::string currentWorkDir_;        /** eg: /home/root/      */
    ProcessSettings settings_;
};

}


#endif //OSADAPTOR_PROCESS_PROCESS_H
