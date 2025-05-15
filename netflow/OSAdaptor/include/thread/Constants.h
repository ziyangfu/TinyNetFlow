
#ifndef OSADAPTOR_THREAD_CONSTANTS_H
#define OSADAPTOR_THREAD_CONSTANTS_H

#include <cstdint>

namespace osadaptor::thread {

constexpr std::size_t kMaxThreadNameLength {15};    /** pthread_setname_np 最大长度为15个字节 */
constexpr std::size_t kDefaultThreadStackSize {0};   /** 可以配置线程栈大小 */
constexpr std::size_t kMaxThreadStackSize {1024 * 1024 * 2};   /** 最大栈大小 2 MB */
}

#endif //OSADAPTOR_THREAD_CONSTANTS_H
