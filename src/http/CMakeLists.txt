file(GLOB_RECURSE HTTP_SRCS ${PROJECT_SOURCE_DIR}/src/http/*.cpp)
file(GLOB_RECURSE HTTP_HEADER ${PROJECT_SOURCE_DIR}/src/http/*.h)
file(GLOB_RECURSE HTTP_TOTAL_SRCS ${HTTP_SRCS} ${HTTP_HEADER})

add_library(netflow_http STATIC ${HTTP_TOTAL_SRCS})
target_link_libraries(netflow_http ${NETFLOW})

if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()