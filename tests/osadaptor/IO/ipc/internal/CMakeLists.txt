cmake_minimum_required(VERSION 3.16)
project(osadaptor_shm_internal_test)

find_package(GTest REQUIRED)

set(relativeIncludePath ../../../../../netflow/OSAdaptor/include/)
set(relativeSrcPath ../../../../../netflow/OSAdaptor/src)
include_directories(${relativeIncludePath})

add_executable(RingBufferTest ./RingBufferTest.cpp
                                ${relativeSrcPath}/IO/ipc/internal/RingBuffer.cpp
)
target_link_libraries(RingBufferTest GTest::gtest_main)
