
#ifndef OSADAPTOR_THREAD_CONSTANTS_H
#define OSADAPTOR_THREAD_CONSTANTS_H

#include <cstdint>

namespace osadaptor::thread {

constexpr std::size_t kMaxThreadNameLength {30};
constexpr std::size_t kDefaultThreadStackSize {0};   /** 可以配置线程栈大小 */
}

#endif //OSADAPTOR_THREAD_CONSTANTS_H
