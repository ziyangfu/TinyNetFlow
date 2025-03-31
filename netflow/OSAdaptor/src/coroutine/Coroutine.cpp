/*!
 * \brief
 * */
#include "coroutine/coroutine.h"

namespace osadaptor::coroutine {

Coroutine::Coroutine(int size, char *stackPtr) {

}

Coroutine::Coroutine(int size, char *stackPtr, std::function<void()> callback) {

}


Coroutine::~Coroutine() {

}


void Coroutine::setCallback(std::function<void()> callback) {

}


void Coroutine::resume(osadaptor::coroutine::Coroutine *coroutine) {

}

void Coroutine::yield() {

}


Coroutine *Coroutine::getCurrentCoroutine() {

}

}  // namespace osadaptor::coroutine