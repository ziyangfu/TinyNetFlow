
#include "process/Process.h"
#include <unistd.h>          /** 添加头文件以使用 sysconf 函数 */
#include <sched.h>           /** 添加头文件以使用 sched_setscheduler 和 sched_param */
#include <sys/resource.h>    /** 添加头文件以使用 setpriority 和 PRIO_PROCESS */
#include <sys/types.h>       /** 添加头文件以使用 pid_t */
#include <sys/wait.h>
#include <vector>
#include "spdlog/spdlog.h"

namespace osadaptor::process {

/** static */ int Process::processCount_ {0};

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
        if (!configureScheduler()) {
            SPDLOG_ERROR("Failed to configure scheduler, current policy is {}, "
                         "set RR/FIFO need root, please check", schedulerPolicyToString(SchedulerPolicy::RR));
            exit(EXIT_FAILURE);
        }
        if (settings_.cpuAffinity_.has_value()) {
            setCpuAffinity();
        }
//        Cgroup cgroup("cpu", "my_cgroup");
//        if (!cgroup.create()) {
//            SPDLOG_ERROR("Failed to create cgroup");
//            exit(EXIT_FAILURE);
//        }
//        if (!cgroup.addProcess(getpid())) {
//            SPDLOG_ERROR("Failed to add process to cgroup");
//            exit(EXIT_FAILURE);
//        }
        if (chdir(currentWorkDir_.c_str()) == -1) {
            SPDLOG_ERROR("Failed to change directory");
            exit(EXIT_FAILURE);
        }

//        if (settings_.supplementaryGroups.has_value()) {
//            if (setgroups(settings_.supplementaryGroups.value().size(), settings_.supplementaryGroups.value().data()) == -1) {
//                SPDLOG_ERROR("Failed to set supplementary groups: {}", strerror(errno));
//                exit(EXIT_FAILURE);
//            }
//        }
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
std::string &Process::getCurrentWorkDir() {
    /** TODO： 当前工作目录如何定义 */
    return currentWorkDir_;
}

void Process::updateProcessSettings(const osadaptor::process::ProcessSettings &newSettings) {
    settings_ = newSettings;
}

void Process::updateArgs(const std::vector<std::string> &newArgs) {
    args_ = newArgs;
}

void Process::updateCurrentWorkDir(const std::string &newDir) {
    currentWorkDir_ = newDir;
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

/*!
 * \brief 当前二进制程序创建了多少个进程
 * \FIXME
 * */
int Process::getProcessCount() {
    /** unordered_map config.size ? */
    return processCount_;
}

/** private **************************************************************************************/
/*!
* \brief 设置CPU亲和性
* */
void Process::setCpuAffinity() {
    if (settings_.cpuAffinity_.has_value()) {
        if (isValidCpuSet(settings_.cpuAffinity_.value(), getSystemCpuCoreCount()) ){
            SPDLOG_ERROR("Invalid CPU affinity value");
            return;
        }
        /** FIXME: 0 代表当前进程？ 应该给到 target pid_t */
        if (sched_setaffinity(0, sizeof(cpu_set_t), &settings_.cpuAffinity_.value()) == -1) {
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
 *      sched_setscheduler 设置实时调度策略与调度优先级（0 ~ 99）， 需root
 *      setpriority(PRIO_PROCESS ... 设置单个进程的动态优先级, 普通策略（-20 ~ 19）
 * */
bool Process::configureScheduler() {
    sched_param param { .sched_priority = settings_.schePriority_ };

    int policy_int = static_cast<int>(settings_.schePolicy_);
    if (sched_setscheduler(processPid_, policy_int, &param) == -1) {
        SPDLOG_ERROR("Error setting scheduler policy");
        return false;
    }
    if (setpriority(PRIO_PROCESS, static_cast<id_t>(processPid_), settings_.schePriority_) == -1) {
        SPDLOG_ERROR("Error setting process priority");
        return false;
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
/** static */ bool Process::isValidCpuSet(const cpu_set_t &cpuSet, int numCpus) {
    for (int i = 0; i < numCpus; ++i) {
        if (CPU_ISSET(i, &cpuSet)) {
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
}  // namespace osadaptor::process