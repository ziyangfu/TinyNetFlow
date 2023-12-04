message(STATUS "enable POSIX shared memory!")
file(GLOB IPC_SHM_SRCS *cpp)
file(GLOB IPC_SHM_HEADERS *.h)
set(IPC_SHM_SRCS ${IPC_SHM_SRCS} CACHE INTERNAL "POSIX shared memory")