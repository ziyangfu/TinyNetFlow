set(BASE_DIR ${PROJECT_SOURCE_DIR}/src/base)
file(GLOB_RECURSE BASE_SRCS ${BASE_DIR}/*.cpp)

add_library(muduo_base ${BASE_SRCS})
target_link_libraries(muduo_base pthread rt)
