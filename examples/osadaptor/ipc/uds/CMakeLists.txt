#find_package(osadaptor REQUIRED)
#if (osadaptor_FOUND)
#    message(STATUS "OSAdaptor found")
#endif ()
#find_package(spdlog REQUIRED)
#if (spdlog_FOUND)
#    message(STATUS "spdlog found")
#endif ()
include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")
add_executable(UdsServerSample UdsServerSample.cpp)
target_link_libraries(UdsServerSample osadaptor)
install(TARGETS UdsServerSample
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/apps)

add_executable(UdsClientSample UdsClientSample.cpp)
target_link_libraries(UdsClientSample osadaptor)
install(TARGETS UdsClientSample
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/apps)

add_executable(UdsClientForShmSample UdsClientForShmSample.cpp )
target_link_libraries(UdsClientForShmSample osadaptor)
install(TARGETS UdsClientForShmSample
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/apps)

add_executable(UdsServerForShmSample UdsServerForShmSample.cpp UdsServerForShmSample.cpp )
target_link_libraries(UdsServerForShmSample osadaptor)

install(TARGETS UdsServerForShmSample
                RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/apps)