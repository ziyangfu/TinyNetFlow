cmake_minimum_required(VERSION 3.16)
project(osadaptor_shm_test)

add_executable(IpcProtocolHeaderTest ./IpcProtocolHeaderTest.cpp
../../../../../netflow/OSAdaptor/include/IO/ipc/internal/IpcProtocolHeader.h
)
