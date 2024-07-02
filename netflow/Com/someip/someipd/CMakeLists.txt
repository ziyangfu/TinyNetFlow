cmake_minimum_required(VERSION 3.5)
project(someipd VERSION 0.1.0)

set(EXECUTABLE_NAME "someipd")

set(CMAKE_CXX_FLAGS "-Wall 
                    -Wextra 
                    -Wpedantic
                    -Werror
                    -Wno-unused-parameter
                    -Wno-unused-variable
                    -Wno-unused-function
                    -Wno-unused-but-set-variable
                    -Wno-unused-but-set-parameter
                    -Wno-unused-local-typedefs 
                    -Wno-unused-result 
                    -Wno-unused-")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(Threads REQUIRED)

file(SOMEIPD_FILES GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${EXECUTABLE_NAME} ${SOMEIPD_FILES})
target_link_libraries(
    osadaptor
)

install()