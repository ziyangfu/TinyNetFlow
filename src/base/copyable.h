//
// Created by fzy on 23-3-16.
//

#ifndef LIBZV_COPYABLE_H
#define LIBZV_COPYABLE_H

namespace muduo {
//! protected: 其他类不可见，子类可见
class Copyable {
protected:
    Copyable() = default;
    ~Copyable() = default;
};

}  // namespace muduo


#endif //LIBZV_COPYABLE_H
