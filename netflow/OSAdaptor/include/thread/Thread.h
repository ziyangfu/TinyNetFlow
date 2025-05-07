/*!
 * \brief 线程的资源管理， 基于pthread，部分参照了 std::thread的写法
 *
 * */

#ifndef OSADAPTOR_THREAD_THREAD_H
#define OSADAPTOR_THREAD_THREAD_H

#include <pthread.h>
#include <cstdint>
#include <string>
#include <optional>


namespace osadaptor::thread {

class Thread final {
public:

    template<typename _Callable,
             std::enable_if_t<!std::is_same<std::decay_t<_Callable>, std::function<void()> = nullptr,
             typename... Args>
    static void create(int config, _Callable&& func);


    template<typename _Callable, typename... Args>
    static Thread create();


    void join();
    bool joinable() const noexcept;

    void detach();

    void getThreadName();
    void setThreadName();
    int getThreadId() const noexcept;



    static Thread startThread();

private:

    void swap();

private:
    pthread_t threadId_;

    std::optional<std::size_t> threadStackSize_;  /** 可配置 */
    std::string threadName_;



};
}

/*
pthread_setaffinity_np

    choreography_conf {
        choreography_processor_num: 8
        choreography_affinity: "range"
        choreography_cpuset: "0-7" # bind CPU cores
        choreography_processor_policy: "SCHED_FIFO" # policy: SCHED_OTHER,SCHED_RR,SCHED_FIFO
 */



#endif //OSADAPTOR_THREAD_THREAD_H
