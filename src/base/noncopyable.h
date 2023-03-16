//
// Created by fzy on 23-3-13.
//

/*! 禁止拷贝构造与赋值构造 */

#ifndef MUDUO_NONCOPYABLE_H
#define MUDUO_NONCOPYABLE_H

namespace muduo {

class Noncopyable {
public:
    Noncopyable(const Noncopyable&) = delete;
    void operator=(const Noncopyable&) = delete;
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;

};
} // muduo

#endif //LIBZV_NONCOPYABLE_H
