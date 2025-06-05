/*
shared_ptr和unique_ptr的简易实现
*/

#include <iostream>

template<class T>
class myShared_ptr {
public:
explicit myShared_ptr(T* ptr = nullptr) 
    : m_ptr(ptr), 
      m_count(ptr ? new size_t(1) : nullptr) 
{
}
~myShared_ptr() {
    release();

}
//拷贝构造实现
myShared_ptr(const myShared_ptr& other) 
    : m_ptr(other.m_ptr),
    m_count(other.m_count) 
{
    if (m_count)
    {
        ++(*m_count);
    }
    
}
// 赋值构造实现
myShared_ptr& operator=(const myShared_ptr& other) {
    if (this != &other)
    {
        /**
        拷贝构造函数：不需要调用 release()，因为它用于创建一个新的对象，并共享现有的资源。
        赋值构造函数：需要调用 release()，因为它用于将当前对象的资源替换为新的资源，
        确保不会出现内存泄漏。
        */
        release(); 
        m_ptr = other.m_ptr;
        m_count = other.m_count;
        if (m_count) {
            ++(*m_count);

        }
    }
    return *this;
    
}

T& operator*() const {
    return *m_ptr;
}

T* operator->() const {
    return m_ptr;
}

size_t use_count() const {
    return m_count ? *m_count : 0;
}
T* get() const{
    return m_ptr;
}
private:
    void release() {
        if (m_count && --(*m_count) == 0)
        {
            delete m_ptr;
            delete m_count;
        }
    }
private:
    T* m_ptr;
    size_t* m_count;
};

template<class T>
class myUnique_ptr {
public:
explicit myUnique_ptr(T* ptr = nullptr) 
    : m_ptr(ptr) 
{
}
~myUnique_ptr() {
    delete m_ptr;
}

myUnique_ptr(const myUnique_ptr&) = delete;
myUnique_ptr& operator=(const myUnique_ptr&) = delete;

myUnique_ptr(myUnique_ptr&& other) noexcept
    : m_ptr(other.m_ptr) {
      other.m_ptr = nullptr;
}

myUnique_ptr& operator=(myUnique_ptr&& other) noexcept {
    if (this != &other)
    {
        delete m_ptr;
        m_ptr = other.m_ptr;
        other.m_ptr = nullptr;
    }
    return *this;
}

T& operator*() const {
    return *m_ptr;
}
T* operator->() const {
    return m_ptr;
}
T* get() const{
    return m_ptr;
}

explicit operator bool() const {
    return m_ptr != nullptr;
}


private:
    T* m_ptr;
};

class A {
public:
    A() {
        std::cout << "A()" << std::endl;
    }
    ~A() {
        std::cout << "~A()" << std::endl;
    }

    void func() {
        std::cout << "func()" << std::endl;
    }
};
int main() {
    myShared_ptr<A> ptr(new A());

    {
        myShared_ptr<A> ptr2 = ptr;
        ptr->func();
        ptr2->func();
        std::cout << "ptr.use_count() = " << ptr.use_count() << std::endl;
    }
    std::cout << "ptr.use_count() = " << ptr.use_count() << std::endl;

    std::cout << "------------------------------" << std::endl;

    myUnique_ptr<A> uptr(new A());
    uptr->func();

    //myUnique_ptr<A> uptr2(uptr);
    //myUnique_ptr<A> uptr3 = uptr;
    //uptr3 = uptr;

    myUnique_ptr<A> uptr2(std::move(uptr));
    uptr2->func();
    myUnique_ptr<A> uptr3;
    uptr3 = std::move(uptr2);
    uptr3->func();

    return 0;

}
