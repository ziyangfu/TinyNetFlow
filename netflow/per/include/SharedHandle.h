/*!
 * \brief based on shared_ptr, provide a simple interface to access key-value storage
 * */

#ifndef PER_SHARED_HANDLE_H
#define PER_SHARED_HANDLE_H

#include <memory>



class SharedHandle {
public:
    SharedHandle() = default;

    SharedHandle(const SharedHandle&) = delete;

    SharedHandle(SharedHandle&&) = delete;

    SharedHandle& operator=(const SharedHandle&) = delete;

    SharedHandle& operator=(SharedHandle&&) = delete;

    ~SharedHandle() = default;

    void createKeyValueStorage();

    void resetKeyValueStorage();

    void openKeyValueStorage();

    void recoverKeyValueStorage();

    std::uint64_t getCurrentKeyValueStorageSize();
};


#endif //TINYNETFLOW_PERTEMP_H
