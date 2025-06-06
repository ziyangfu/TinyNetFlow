
#include "process/Process.h"
#include <unistd.h>          /** 添加头文件以使用 sysconf 函数 */
#include <sched.h>           /** 添加头文件以使用 sched_setscheduler 和 sched_param */
#include <sys/resource.h>    /** 添加头文件以使用 setpriority 和 PRIO_PROCESS */
#include <sys/types.h>       /** 添加头文件以使用 pid_t */
#include <sys/wait.h>
#include <vector>
#include "spdlog/spdlog.h"
#include "process/CGroupV2Controller.h"

#include "BpfTrace.h"
namespace osadaptor::process {

/** static */ bool isSetIsolatedCpu_ = false;
/** static */ std::vector<int> isolatedCpus_;

Process::Process(std::string programPath,
                 std::vector<std::string> args,
                 osadaptor::process::ProcessSettings settings)
         : programPath_(std::move(programPath)),
           args_(std::move(args)),
           settings_(std::move(settings))
{
    bool isExecutable = checkExecutable();
    if (!isExecutable) {
        SPDLOG_ERROR("{} is not executable", programPath);
        return;
    }
}

Process::~Process() {
}

/*!
 * \brief 进程创建
 * \details
        // 通过fork系统调用创建子进程
        pid{::fork()};
        // 父进程
        if (pid > 0) {
          // 父进程啥也不干
        }
        // 子进程
        else {
          // cgroup资源设置，cgroups
          // 将pid写cgroup资源组
          cgroup.AddPid(pid);
          // 调度策略及优先级设置，底层通过 sched_setscheduler系统调用
          SetSchedulerSettings();
          // 修改工作目录
          chdir();
          // 判定CPU隔离核心有效性
          // 底层通过 sched_setaffinity 设置CPU亲和性
          sched_setaffinity();
          // 底层通过setgroups 设置补充组ID
          setgroups();
          //底层通过setregid 设置真实有效的组ID
          setregid(gid, gid);
          //底层通过setreuid 设置真实有效的用户ID
          setreuid(uid, uid);
          //底层通过execve，将子进程替换为全新非父子关系的独立进程
          execve();
 * */
void Process::processCreate() {
    std::vector<char*> CArgs;
    CArgs.push_back(const_cast<char*>(programPath_.c_str()));
    for (const auto& arg : args_) {
        CArgs.push_back(const_cast<char*>(arg.c_str()));
    }
    CArgs.push_back(nullptr);

    pid_t pid = ::fork();
    if (pid == -1) {
        SPDLOG_ERROR("Fork failed");
        exit(EXIT_FAILURE);
    }

    /** 父进程 */
    if (pid > 0) {
        if (spdlog::get_level() == spdlog::level::trace) {
            int status;
            ::waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                SPDLOG_TRACE("Child process {} exited with status {}", program ,WEXITSTATUS(status));
            }
        }
        return;
    }
    /** 子进程 */
    else {
        processPid_ = ::getpid();
        BPF_USER_PROBE_2("osa_process", "process_create", processPid_, settings_);
        if (!configureScheduler()) {
            SPDLOG_ERROR("Failed to configure scheduler, current sche policy is {}, "
                         "set RR/FIFO need root, please check",
                         schedulerPolicyToString(settings_.schePolicy_));
            exit(EXIT_FAILURE);
        }
        if  (settings_.isRunInIsolatedCpu_ == true) {
            if (!isolateIsValid()) {
                SPDLOG_ERROR("Isolate cpu core setting failed, please check");
            }
        }
        if (settings_.cpuAffinity_.has_value()) {
            setCpuAffinity();
        }
        if (settings_.cGroupSettings_.isSettingCGroup == true) {

            if (settings_.cGroupSettings_.resourceGroupPath_.has_value()) {
                CGroupV2Controller cgroup(settings_.cGroupSettings_.resourceGroupPath_.value());
                if (settings_.cGroupSettings_.cpuLimit_.has_value()) {
                    cgroup.setCPULimit(settings_.cGroupSettings_.cpuLimit_.value());
                }
                if (settings_.cGroupSettings_.memoryLimit_.has_value()) {
                    cgroup.setMemoryLimit(settings_.cGroupSettings_.memoryLimit_.value());
                }
                cgroup.addProcess(processPid_);
            }
            else {
                CGroupV2Controller cgroup;
                if (settings_.cGroupSettings_.cpuLimit_.has_value()) {
                    cgroup.setCPULimit(settings_.cGroupSettings_.cpuLimit_.value());
                }
                if (settings_.cGroupSettings_.memoryLimit_.has_value()) {
                    cgroup.setMemoryLimit(settings_.cGroupSettings_.memoryLimit_.value());
                }
                cgroup.addProcess(processPid_);
            }
        }
        if (settings_.currentWorkDir_.has_value()) {
            if (chdir(settings_.currentWorkDir_.value().c_str()) == -1) {
                SPDLOG_ERROR("Failed to change directory");
            }
        }
        if (settings_.userId_.has_value()) {
            if (setreuid(settings_.userId_.value(), settings_.userId_.value()) == -1) {
                SPDLOG_ERROR("Failed to set real and effective user ID: {}", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        if (settings_.userGroupId_.has_value()) {
            if (setregid(settings_.userGroupId_.value(), settings_.userGroupId_.value()) == -1) {
                SPDLOG_ERROR("Failed to set real and effective group ID: {}", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        if (execve(CArgs[0], CArgs.data(), nullptr) == -1) {
            SPDLOG_ERROR("Execve failed");
            exit(EXIT_FAILURE);
        }
    }
}

/*!
 * \brief 获取程序名，第一次用时才从path中切出name
 * */
std::string &Process::getProgramName() {
    if (programName_.empty()) {
        std::size_t const index {programPath_.find_last_of('/')};
        if (index != std::string::npos) {
            programName_ = programPath_.substr(index + 1, std::string::npos);
        }
    }
    return programName_;
}

/*!
 * \brief 获取当前工作目录
 * */
std::string Process::getCurrentWorkDir() {
    /** TODO： 当前工作目录如何定义 */
    return "";
}

void Process::updateProcessSettings(const osadaptor::process::ProcessSettings &newSettings) {
    settings_ = newSettings;
}

void Process::updateArgs(const std::vector<std::string> &newArgs) {
    args_ = newArgs;
}

void Process::updateCurrentWorkDir(const std::string &newDir) {
    //currentWorkDir_ = newDir;
}

ProcessSettings &Process::getCurrentProcessSettings() {
    return settings_;
}

std::string Process::getCurrentSchedulePolicyStr() {
    return schedulerPolicyToString(settings_.schePolicy_);
}

pid_t Process::getCurrentProcessPid() const {
    return processPid_;
}


void Process::sendSIGKILL() {

}

void Process::sendSIGTERM() {

}


void Process::setIsolatedCpuCount(int num) {
    if (num <= 0 || num > kDefaultIsolatedCpuCountMax) {
        SPDLOG_ERROR("Invalid CPU count, current max isolated CPU count is {}",
                     kDefaultIsolatedCpuCountMax);
    }
    // set isolated cpu
    if  (num == 1) {
        isolatedCpus_.push_back(0);
    }
    else if (num == 2) {
        isolatedCpus_.push_back(1);
    }
    if (isolatedCpuManager::isolateCpus(isolatedCpus_)) {
        isSetIsolatedCpu_ = true;
    }
    else {
        isSetIsolatedCpu_ = false;
        SPDLOG_ERROR("Failed to set isolated cpu");
    }
}

void Process::unsetIsolatedCpu() {

}

bool Process::isSetIsolatedCpu() {
    return isSetIsolatedCpu_;
}

/*!
 * \details
 *      0. 超级权限root
 *      1. isSetIsolatedCpu_ CPU隔离已设置
 *      2. 若隔离核心为1，则cpuAffinity_必须设置为0
 *      3. 若隔离核心为2， 则cpuAffinity必须设置为0或1
 * */
bool Process::isolateIsValid() {
    bool result {false};
    if (isSetIsolatedCpu_ && isSuperuserPrivileges()) {
        if (isolatedCpus_.size() == 1) {
            if (settings_.cpuAffinity_.has_value() && settings_.cpuAffinity_.value() == 0) {
                result = true;
            }
        }
        else if (isolatedCpus_.size() == 2) {
            if (settings_.cpuAffinity_.has_value() &&
                    (settings_.cpuAffinity_.value() == 0 || settings_.cpuAffinity_.value() == 1)) {
                result = true;
            }
        }
    }
    return result;
}

bool Process::isCpuInIsolatedList(int cpu) {
    bool result {false};
    if (isolatedCpus_.empty()) {
        return result;
    }
    result = std::any_of(isolatedCpus_.begin(), isolatedCpus_.end(),
                [cpu](int c) { return c == cpu;});
    return result;
}

std::vector<int> &Process::getIsolatedCpus() {
    return isolatedCpus_;
}

std::string Process::getIsolatedCpusString() {
    if (isolatedCpus_.empty()) {
        return "";
    }
    return fmt::format("{}", fmt::join(isolatedCpus_, ","));
}

bool Process::checkIsolateCpuWithSystemSetting() {
    /** 是否有顺序问题？ 如果有顺序,则需要先排序 */
    auto v1 = isolatedCpuManager::getIsolatedCpusFromSystem();
    std::sort(v1.begin(), v1.end());
    std::sort(isolatedCpus_.begin(), isolatedCpus_.end());
    return isolatedCpus_ == v1;
    /** 如果没有顺序问题，则只需要==即可 */
//    return isolatedCpus_ == isolatedCpuManager::getIsolatedCpusFromSystem();
}

/** private **************************************************************************************/
/*!
* \brief 设置CPU亲和性
 * \details
 *      0：当前进程
* */
void Process::setCpuAffinity() {
    if (settings_.cpuAffinity_.has_value()) {
        if (isValidCpuSet(settings_.cpuAffinity_.value(), getSystemCpuCoreCount()) ){
            SPDLOG_ERROR("Invalid CPU affinity value");
            return;
        }

        if (sched_setaffinity(0, sizeof(cpu_set_t),
                              reinterpret_cast<const cpu_set_t*>(&settings_.cpuAffinity_.value())) == -1) {
            SPDLOG_ERROR("Error setting CPU affinity");
            return;
        }
    }
    else {
        SPDLOG_ERROR("No CPU affinity specified, please give a value of CPU affinity in settings");
    }
}

/*!
 * \brief
 * \details
 *      sched_setscheduler 设置实时调度策略与调度优先级， sched_priority范围：1 ~ 99， 0表示非实时， 需root
 *      setpriority(PRIO_PROCESS ... 设置单个进程的动态优先级 nice值, 普通策略（-20 ~ 19）
 * */
bool Process::configureScheduler() {
    if (settings_.schePolicy_ == SchedulerPolicy::OTHER ||
        settings_.schePolicy_ == SchedulerPolicy::BATCH ||
        settings_.schePolicy_ == SchedulerPolicy::IDLE) {
        if (settings_.schePriority_ < -20 || settings_.schePriority_ > 19) {
            SPDLOG_ERROR("Invalid priority value, please give a value between -20 and 19");
            return false;
        }
        if (setpriority(PRIO_PROCESS, static_cast<id_t>(processPid_), settings_.schePriority_) == -1) {
            SPDLOG_ERROR("Error setting process priority");
            return false;
        }
    }
    else if (settings_.schePolicy_ == SchedulerPolicy::FIFO ||
             settings_.schePolicy_ == SchedulerPolicy::RR) {
        /** 1 ~ 99 in Linux AMD64 */
        if (settings_.schePriority_ < getCurrentSysRtPriorityMin() ||
            settings_.schePriority_ > getCurrentSysRtPriorityMax()) {
            SPDLOG_ERROR("Invalid priority value, please give a value in {} ~ {}",
                         getCurrentSysRtPriorityMin(), getCurrentSysRtPriorityMax());
            return false;
        }
        sched_param param { .sched_priority = settings_.schePriority_ };
        const int policy_int = static_cast<int>(settings_.schePolicy_);
        if (sched_setscheduler(0, policy_int, &param) == -1) {
            SPDLOG_ERROR("sched_setscheduler failed: {} (errno={})", strerror(errno), errno);
            return false;
        }
    }
    SPDLOG_TRACE("Scheduler policy set to {} with priority {}", schedulerPolicyToString(policy), priority);
    return true;
}

/*!
 * \brief 获取 CPU 核心数，至少有一个核心
 * */
/** static */ int Process::getSystemCpuCoreCount()
{
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores == -1) {
        return 1;
    }
    return static_cast<int>(num_cores);
}

/*!
 * \brief 判断欲设置的CPU核心是否是有效的核心，即CPU 核心数内
 * \return true 表示有效，false 表示无效
 * */
/** static */ bool Process::isValidCpuSet(const std::uint32_t cpuSet, int numCpus) {
    for (int i = 0; i < numCpus; ++i) {
        if (CPU_ISSET(i, reinterpret_cast<const cpu_set_t*>(&cpuSet))) {
            return true;
        }
    }
    return false;
}

/*!
 * \brief 返回调度策略对应的字符串
 * */
/** static */ std::string Process::schedulerPolicyToString(osadaptor::process::SchedulerPolicy policy) {
    switch (policy) {
        case SchedulerPolicy::OTHER:
            return "SCHED_OTHER";
        case SchedulerPolicy::FIFO:
            return "SCHED_FIFO";
        case SchedulerPolicy::RR:
            return "SCHED_RR";
        case SchedulerPolicy::BATCH:
            return "SCHED_BATCH";
        case SchedulerPolicy::IDLE:
            return "SCHED_IDLE";
        case SchedulerPolicy::DEADLINE:
            return "SCHED_DEADLINE";
        default:
            return "UNKNOWN";
    }
}

/*!
 * \brief 是否具有超级用户权限
 * */
/** FIXME: 检查的是当前程序，如execmd程序是否具有超级权限，不一定要在这里确定， 在execmd中确定更好 */
/** static */ bool Process::isSuperuserPrivileges() {
    return ::geteuid() == 0;
}

/*!
 * \brief 检测给定的文件路径是否是可执行文件
 * \details
 *      1. 检查文件存在性
 *      2. 检查是否为常规文件（非目录/符号链接等）
 *      3. 检查可执行权限（至少有一个执行权限位）
 * */
bool Process::checkExecutable() {
    if (access(programPath_.c_str(), F_OK) == -1) {
        SPDLOG_ERROR("File {} does not exist: {}", programPath_, strerror(errno));
        return false;
    }
    struct stat st {};
    if (stat(programPath_.c_str(), &st) == -1) {
        SPDLOG_ERROR("Failed to stat file {}: {}", programPath_, strerror(errno));
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        SPDLOG_ERROR("{} is not a regular file", programPath_);
        return false;
    }
    if (access(programPath_.c_str(), X_OK) == -1) {
        SPDLOG_ERROR("File {} is not executable: {}", programPath_, strerror(errno));
        return false;
    }
    SPDLOG_DEBUG("File {} is valid executable", programPath_);
    return true;
}


/** static */int Process::getCurrentSysRtPriorityMax() {
    return sched_get_priority_max(SCHED_FIFO);  /** SCHED_FIFO 与 SCHED_RR 一致 */
}

/** static */int Process::getCurrentSysRtPriorityMin() {
    return sched_get_priority_min(SCHED_FIFO);
}

}  // namespace osadaptor::process