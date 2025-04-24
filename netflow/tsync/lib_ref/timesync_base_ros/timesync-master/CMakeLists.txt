cmake_minimum_required(VERSION 2.8.9)
project(timesync)

find_package(catkin REQUIRED COMPONENTS dynamic_reconfigure roscpp rospy std_msgs message_generation)
set(CMAKE_CXX_FLAGS "-std=c++11")

set(timesync_SRCS
        src/HoltWinters.cpp
        src/TimestampSynchronizer.cpp)

add_message_files(
        FILES
        TimesyncDebug.msg)

generate_messages(
        DEPENDENCIES
        std_msgs
)

# Declare this project as a catkin package
catkin_package(CATKIN_DEPENDS dynamic_reconfigure roscpp message_runtime
               INCLUDE_DIRS include
               LIBRARIES timesync)

include_directories(include/timesync ${catkin_INCLUDE_DIRS})

# Define a library target called hello_world
add_library(timesync ${timesync_SRCS})
target_link_libraries(timesync ${catkin_LIBRARIES})
add_dependencies(timesync ${${PROJECT_NAME}_EXPORTED_TARGETS})

add_executable(example src/example.cpp)
target_link_libraries(example ${catkin_LIBRARIES} timesync)
