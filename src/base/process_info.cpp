//
// Created by fzy on 23-3-30.
//
/*!
#include <assert.h>：用于程序中的断言assert()函数，用于检查程序中的某个条件是否为真。如果条件为假，则输出一条错误消息，并且使程序终止。
#include <dirent.h>：用于访问目录和文件。包含了一些函数和数据结构，例如opendir()、readdir()和closedir()函数等。
#include <pwd.h>：用于访问用户和组信息。包含了一些函数和数据结构，例如getpwuid()、getpwnam()和passwd结构体等。
#include <stdio.h>：用于标准输入输出的库函数，包含了一些输入输出函数，例如fopen()、fclose()、fread()、fwrite()等，还包括了一些
                    格式化输出函数，例如sprintf()和snprintf()等。
#include <stdlib.h>：包含了一些通用的函数，例如malloc()、free()、atoi()和exit()等，这些函数可用于内存分配、字符串转换和程序退出等操作。
#include <unistd.h>：用于实现各种与POSIX操作系统交互的函数，例如fork()、exec()、pipe()和sleep()等。
#include <sys/resource.h>：包含了一些系统资源的操作函数和数据结构，例如getrlimit()、setrlimit()和rlimit结构体等，这些函数和数据
                           结构可用于限制进程的资源使用。
#include <sys/times.h>：包含了一些操作进程时间的函数和数据结构，例如times()和tms结构体等，这些函数和数据结构可用于测量进程的CPU时间。
 */
#include "process_info.h"
#include "current_thread.h"
#include "file_util.h"

#include <algorithm>

#include <assert.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h> // snprintf
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>

namespace muduo {
namespace detail {
__thread int t_numOpenedFiles = 0;
int fdDirFilter(const struct dirent* d)
{
    if (::isdigit(d->d_name[0]))
    {
        ++t_numOpenedFiles;
    }
    return 0;
}

__thread std::vector<pid_t>* t_pids = NULL;
int taskDirFilter(const struct dirent* d)
{
    if (::isdigit(d->d_name[0]))
    {
        t_pids->push_back(atoi(d->d_name));
    }
    return 0;
}

int scanDir(const char *dirpath, int (*filter)(const struct dirent *))
{
    struct dirent** namelist = NULL;
    int result = ::scandir(dirpath, &namelist, filter, alphasort);
    assert(namelist == NULL);
    return result;
}

Timestamp g_startTime = Timestamp::now();
// assume those won't change during the life time of a process.
int g_clockTicks = static_cast<int>(::sysconf(_SC_CLK_TCK));
int g_pageSize = static_cast<int>(::sysconf(_SC_PAGE_SIZE));
} // namespace detail
} // namespace muduo


using namespace muduo;
using namespace muduo::detail;

pid_t ProcessInfo::pid()
{
    return ::getpid();
}

string ProcessInfo::pidString()
{
    char buf[32];
    snprintf(buf, sizeof buf, "%d", pid());
    return buf;
}

uid_t ProcessInfo::uid()
{
    return ::getuid();
}

string ProcessInfo::username()
{
    struct passwd pwd;
    struct passwd* result = NULL;
    char buf[8192];
    const char* name = "unknownuser";

    getpwuid_r(uid(), &pwd, buf, sizeof buf, &result);
    if (result)
    {
        name = pwd.pw_name;
    }
    return name;
}

uid_t ProcessInfo::euid()
{
    return ::geteuid();
}

Timestamp ProcessInfo::startTime()
{
    return g_startTime;
}

int ProcessInfo::clockTicksPerSecond()
{
    return g_clockTicks;
}

int ProcessInfo::pageSize()
{
    return g_pageSize;
}

bool ProcessInfo::isDebugBuild()
{
#ifdef NDEBUG
    return false;
#else
    return true;
#endif
}

string ProcessInfo::hostname()
{
    // HOST_NAME_MAX 64
    // _POSIX_HOST_NAME_MAX 255
    char buf[256];
    if (::gethostname(buf, sizeof buf) == 0)
    {
        buf[sizeof(buf)-1] = '\0';
        return buf;
    }
    else
    {
        return "unknownhost";
    }
}

string ProcessInfo::procname()
{
    return procname(procStat()).as_string();
}

StringPiece ProcessInfo::procname(const string& stat)
{
    StringPiece name;
    size_t lp = stat.find('(');
    size_t rp = stat.rfind(')');
    if (lp != string::npos && rp != string::npos && lp < rp)
    {
        name.set(stat.data()+lp+1, static_cast<int>(rp-lp-1));
    }
    return name;
}

string ProcessInfo::procStatus()
{
    string result;
    FileUtil::readFile("/proc/self/status", 65536, &result);
    return result;
}

string ProcessInfo::procStat()
{
    string result;
    FileUtil::readFile("/proc/self/stat", 65536, &result);
    return result;
}

string ProcessInfo::threadStat()
{
    char buf[64];
    snprintf(buf, sizeof buf, "/proc/self/task/%d/stat", CurrentThread::tid());
    string result;
    FileUtil::readFile(buf, 65536, &result);
    return result;
}

string ProcessInfo::exePath()
{
    string result;
    char buf[1024];
    ssize_t n = ::readlink("/proc/self/exe", buf, sizeof buf);
    if (n > 0)
    {
        result.assign(buf, n);
    }
    return result;
}

int ProcessInfo::openedFiles()
{
    t_numOpenedFiles = 0;
    scanDir("/proc/self/fd", fdDirFilter);
    return t_numOpenedFiles;
}

int ProcessInfo::maxOpenFiles()
{
    struct rlimit rl;
    if (::getrlimit(RLIMIT_NOFILE, &rl))
    {
        return openedFiles();
    }
    else
    {
        return static_cast<int>(rl.rlim_cur);
    }
}

ProcessInfo::CpuTime ProcessInfo::cpuTime()
{
    ProcessInfo::CpuTime t;
    struct tms tms;
    if (::times(&tms) >= 0)
    {
        const double hz = static_cast<double>(clockTicksPerSecond());
        t.userSeconds = static_cast<double>(tms.tms_utime) / hz;
        t.systemSeconds = static_cast<double>(tms.tms_stime) / hz;
    }
    return t;
}

int ProcessInfo::numThreads()
{
    int result = 0;
    string status = procStatus();
    size_t pos = status.find("Threads:");
    if (pos != string::npos)
    {
        result = ::atoi(status.c_str() + pos + 8);
    }
    return result;
}

std::vector<pid_t> ProcessInfo::threads()
{
    std::vector<pid_t> result;
    t_pids = &result;
    scanDir("/proc/self/task", taskDirFilter);
    t_pids = NULL;
    std::sort(result.begin(), result.end());
    return result;
}