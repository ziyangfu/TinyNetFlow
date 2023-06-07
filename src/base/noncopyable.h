//
// Created by fzy on 23-3-13.
//

/*! 禁止拷贝构造与赋值构造 */

#ifndef LIBZV_NONCOPYABLE_H
#define LIBZV_NONCOPYABLE_H

namespace netflow {

class Noncopyable {
public:
    Noncopyable(const Noncopyable&) = delete;  /** 禁止拷贝构造 */
    void operator=(const Noncopyable&) = delete; /** 禁止拷贝赋值 */
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;

};
} // libzv

#endif //LIBZV_NONCOPYABLE_H
