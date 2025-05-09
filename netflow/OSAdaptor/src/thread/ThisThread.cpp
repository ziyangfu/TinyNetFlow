#include "thread/ThisThread.h"
#include <pthread.h>

namespace osadaptor::thread {

std::string thisThread::getName() {
    std::string threadName{};
    int const ret = pthread_getname_np(pthread_self(), threadName.data(), threadName.size());
    if (ret != 0) {
        threadName = "unknown";
    }
    return threadName;
}

}  // namespace osadaptor::thread