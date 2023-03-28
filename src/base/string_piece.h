//
// Created by fzy on 23-3-28.
//

//! 字符串切片

/*!
 * chatGPT:
 * 这是一个定义了字符串切片的类 StringPiece，可以用于在代码中处理字符串。

该类包括以下成员函数：

构造函数：提供了多个构造函数，用于将字符串或字符数组转化为 StringPiece 对象。
data：返回指向存储 StringPiece 中字符串的指针，注意该指针可能包含 NUL 字符。
size：返回 StringPiece 中字符串的长度。
empty：返回 StringPiece 是否为空。
begin/end：返回指向存储 StringPiece 中字符串的指针，可用于迭代器。
clear：将 StringPiece 重置为空。
set：提供了多个函数，用于将字符串或字符数组赋值给 StringPiece。
operator[]：返回 StringPiece 中字符串中的第 i 个字符。
remove_prefix/remove_suffix：移除 StringPiece 的前缀或后缀。
operator==/!=/<等比较符：比较两个 StringPiece 是否相等。
compare：比较两个 StringPiece 的大小。
as_string：将 StringPiece 转换为 std::string 类型。
CopyToString：将 StringPiece 的字符串复制到 std::string 类型的变量中。
starts_with：检查 StringPiece 是否以给定的字符串开始。
此外，该代码还定义了 StringArg 类，用于在函数中传递字符串参数。还提供了一些函数，用于在 STL 容器中使用 StringPiece。
 */

#ifndef LIBZV_STRING_PIECE_H
#define LIBZV_STRING_PIECE_H

#include <string.h>
#include <iosfwd>  // for ostream forward-declaration

#include "types.h"
namespace muduo
{

// For passing C-style string argument to a function.
    class StringArg // copyable
    {
    public:
        StringArg(const char* str)
                : str_(str)
        { }

        StringArg(const string& str)
                : str_(str.c_str())
        { }

        const char* c_str() const { return str_; }

    private:
        const char* str_;
    };

    class StringPiece {
    private:
        const char*   ptr_;
        int           length_;

    public:
        // We provide non-explicit singleton constructors so users can pass
        // in a "const char*" or a "string" wherever a "StringPiece" is
        // expected.
        StringPiece()
                : ptr_(NULL), length_(0) { }
        StringPiece(const char* str)
                : ptr_(str), length_(static_cast<int>(strlen(ptr_))) { }
        StringPiece(const unsigned char* str)
                : ptr_(reinterpret_cast<const char*>(str)),
                  length_(static_cast<int>(strlen(ptr_))) { }
        StringPiece(const string& str)
                : ptr_(str.data()), length_(static_cast<int>(str.size())) { }
        StringPiece(const char* offset, int len)
                : ptr_(offset), length_(len) { }

        // data() may return a pointer to a buffer with embedded NULs, and the
        // returned buffer may or may not be null terminated.  Therefore it is
        // typically a mistake to pass data() to a routine that expects a NUL
        // terminated string.  Use "as_string().c_str()" if you really need to do
        // this.  Or better yet, change your routine so it does not rely on NUL
        // termination.
        const char* data() const { return ptr_; }
        int size() const { return length_; }
        bool empty() const { return length_ == 0; }
        const char* begin() const { return ptr_; }
        const char* end() const { return ptr_ + length_; }

        void clear() { ptr_ = NULL; length_ = 0; }
        void set(const char* buffer, int len) { ptr_ = buffer; length_ = len; }
        void set(const char* str) {
            ptr_ = str;
            length_ = static_cast<int>(strlen(str));
        }
        void set(const void* buffer, int len) {
            ptr_ = reinterpret_cast<const char*>(buffer);
            length_ = len;
        }

        char operator[](int i) const { return ptr_[i]; }

        void remove_prefix(int n) {
            ptr_ += n;
            length_ -= n;
        }

        void remove_suffix(int n) {
            length_ -= n;
        }

        bool operator==(const StringPiece& x) const {
            return ((length_ == x.length_) &&
                    (memcmp(ptr_, x.ptr_, length_) == 0));
        }
        bool operator!=(const StringPiece& x) const {
            return !(*this == x);
        }

#define STRINGPIECE_BINARY_PREDICATE(cmp,auxcmp)                             \
  bool operator cmp (const StringPiece& x) const {                           \
    int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_); \
    return ((r auxcmp 0) || ((r == 0) && (length_ cmp x.length_)));          \
  }
        STRINGPIECE_BINARY_PREDICATE(<,  <);
        STRINGPIECE_BINARY_PREDICATE(<=, <);
        STRINGPIECE_BINARY_PREDICATE(>=, >);
        STRINGPIECE_BINARY_PREDICATE(>,  >);
#undef STRINGPIECE_BINARY_PREDICATE

        int compare(const StringPiece& x) const {
            int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);
            if (r == 0) {
                if (length_ < x.length_) r = -1;
                else if (length_ > x.length_) r = +1;
            }
            return r;
        }

        string as_string() const {
            return string(data(), size());
        }

        void CopyToString(string* target) const {
            target->assign(ptr_, length_);
        }

        // Does "this" start with "x"
        bool starts_with(const StringPiece& x) const {
            return ((length_ >= x.length_) && (memcmp(ptr_, x.ptr_, x.length_) == 0));
        }
    };

}  // namespace muduo

// ------------------------------------------------------------------
// Functions used to create STL containers that use StringPiece
//  Remember that a StringPiece's lifetime had better be less than
//  that of the underlying string or char*.  If it is not, then you
//  cannot safely store a StringPiece into an STL container
// ------------------------------------------------------------------
//! StringPiece的生命周期最好小于底层字符串或者char*，不然无法安全的将 StringPiece 存入 STL 容器

#ifdef HAVE_TYPE_TRAITS
// This makes vector<StringPiece> really fast for some STL implementations
template<> struct __type_traits<muduo::StringPiece> {
  typedef __true_type    has_trivial_default_constructor;
  typedef __true_type    has_trivial_copy_constructor;
  typedef __true_type    has_trivial_assignment_operator;
  typedef __true_type    has_trivial_destructor;
  typedef __true_type    is_POD_type;
};
#endif

// allow StringPiece to be logged
std::ostream& operator<<(std::ostream& o, const muduo::StringPiece& piece);


#endif //LIBZV_STRING_PIECE_H
