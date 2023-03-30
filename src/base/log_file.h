//
// Created by fzy on 23-3-30.
//

#ifndef LIBZV_LOG_FILE_H
#define LIBZV_LOG_FILE_H

#include "mutex.h"
#include "types.h"

#include <memory>

namespace muduo {

namespace FileUtil {
    class AppendFile;
} // namespace FileUtil

class LogFile : Noncopyable {
public:
    LogFile(const string& basename,
            off_t rollSize,
            bool threadSafe = true,
            int flushInterval = 3,
            int checkEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool rollFile();
private:
    void append_unlocked(const char* logline, int len);
    static string getLogFileName(const string& basename, time_t* now);
private:
    const string basename_;
    const off_t rollSize_;
    const int flushInterval_;
    const int checkEveryN_;

    int count_;

    std::unique_ptr<MutexLock> mutex_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    std::unique_ptr<FileUtil::AppendFile> file_;

    const static int kRollPerSeconds_ = 60*60*24;
};

} // namespace muduo
#endif //LIBZV_LOG_FILE_H
