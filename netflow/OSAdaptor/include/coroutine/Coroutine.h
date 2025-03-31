/*!
 * \brief 协程用户接口
 * */
#ifndef OSADAPTOR_COROUTINE_COROUTINE_H
#define OSADAPTOR_COROUTINE_COROUTINE_H

#include <functional>
#include <string>

namespace osadaptor::coroutine {

class Coroutine {
public:
    Coroutine(int size, char* stackPtr);
    Coroutine(int size, char* stackPtr, std::function<void()> callback);
    ~Coroutine();
    void run();

    void setCallback(std::function<void()> callback);

    static void yield();
    static void resume(Coroutine* coroutine);
    static Coroutine* getCurrentCoroutine();

private:
    int coroutineId_;
    // cortex
    int stackSize_;
    char* stackSp_;
    bool isInCofunc_;
    std::string msgNo_;
};

} // namespace osadaptor::coroutine

#endif //OSADAPTOR_COROUTINE_COROUTINE_H
