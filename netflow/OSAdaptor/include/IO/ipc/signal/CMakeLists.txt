# netflow -> ipc -> signal
message(STATUS "enable netflow -> IPC -> signal")
file(GLOB SIGNAL_SRCS *.cpp)
file(GLOB SIGNAL_HEADERS *.h)
set(IPC_SIGNAL_SRCS CACHE  INTERNAL ${SIGNAL_SRCS} "IPC signal")