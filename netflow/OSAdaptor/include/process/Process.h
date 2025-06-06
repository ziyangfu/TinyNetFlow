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

/*

 嵌入式单片机中有watchdog，嵌入式Linux中有吗？
嵌入式Linux中有watchdog，以守护进程的形式存在，也有硬件级别的watchdog外设。
fzy@fzy-Lenovo:~$ ps -axu | grep watch
root         127  0.0  0.0      0     0 ?        S    09:17   0:00 [watchdogd]

资源管理：
        CPU计算资源 —— 进程管理
        内存 —— 内存管理
        磁盘 —— 文件系统
        电源 —— 电源管理
        外设（driver）
            块设备
                磁盘
            字符设备
                键盘鼠标驱动
            网络设备
                网卡  —— 网络协议栈
            USB驱动
            PCIe驱动
            显示display —— HDMI、DP驱动，
            声卡
        中断管理

从高到低：
 NUMA node
    Zone node（DMA DMA32 NORMAL）
        buddy system
            slab（slub）

slab cache
        很多 slab
            每个slab有很多个object
 * */

#ifndef OSADAPTOR_PROCESS_PROCESS_H
#define OSADAPTOR_PROCESS_PROCESS_H

#include <string>
#include <vector>
#include "process/Types.h"
#include "process/ProcessSetting.h"
#include "process/IsolatedCpuManager.h"
#include "process/ProcessConstants.h"

namespace osadaptor::process {

/** 如何描述一个进程？ 每一个进程存在一个Process实例， 并在生命周期内常驻内存 ？
 * 当收到重启信号时，直接重启
 * 命令参数/程序设定/工作目录 可以修改更新，程序名与程序路径不允许修改
 *
 * 一个二进制程序文件，可以有多个进程实例
 * Process 唯一描述一个进程示例，而不是一个二进制程序文件，创建的多个实例
 * 多个实例由 ProcessCluster描述
 * *
 */
class Process {
public:
    Process(std::string programPath, std::vector<std::string> args,
            ProcessSettings settings);
    ~Process();
    void processCreate();
    std::string& getProgramName();
    std::string getCurrentWorkDir();
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

    /*!
     * \brief 隔离CPU相关，暂时只给设置几个的能力，而不是具体的设置某个CPU逻辑核心
     *       当前最大只给设置2个核心
     * */
     static void setIsolatedCpuCount(int num);
     /*!
      * \brief 取消设置隔离CPU，这个对于全局有效，谨慎使用
      * */
     static void unsetIsolatedCpu();
     /*!
      * \brief 是否设置了隔离CPU核心
      * */
     static bool isSetIsolatedCpu();
    /*!
     * \brief 隔离CPU核心设置是否有效
     * */
     bool isolateIsValid();
    /*!
     * \brief 检查CPU核心是否在隔离列表中
     * */
    static bool isCpuInIsolatedList(int cpu);
    /*!
     * \brief 获取当前隔离中的CPU核心列表
     * */
    static std::vector<int>& getIsolatedCpus();
    /*!
     * \brief 获取当前隔离中的CPU核心列表，以字符串的形式
     * */
    static std::string getIsolatedCpusString();
    /*!
     * \brief 检查静态变量isolatedCpus_是否与系统设置的值一致
     * */
    static bool checkIsolateCpuWithSystemSetting();

private:
    bool checkExecutable();
    void setCpuAffinity();
    bool configureScheduler();
    static int getSystemCpuCoreCount();
    static bool isValidCpuSet(const std::uint32_t cpuSet, int numCpus);
    static bool isSuperuserPrivileges();
    static std::string schedulerPolicyToString(SchedulerPolicy policy);
    /*!
     * \brief 获取当前系统的RT优先级最大值和最小值
     * */
    static int getCurrentSysRtPriorityMax();
    static int getCurrentSysRtPriorityMin();
private:
    pid_t processPid_;                  /** child process pid    */
    std::string programPath_;           /** eg： /usr/bin/ls     */
    std::string programName_;           /** eg: ls               */
    std::vector<std::string> args_;     /** eg： "-a", "-l"       */
    ProcessSettings settings_;
    static bool isSetIsolatedCpu_;     /** 是否设置了隔离CPU核心 */
    static std::vector<int> isolatedCpus_;
};

}

/*
核心难点与技术深度
1. 从0-1的工程搭建
*/


#endif //OSADAPTOR_PROCESS_PROCESS_H
