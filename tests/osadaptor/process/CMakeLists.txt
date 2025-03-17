# osadaptor-process单元测试集合

# 查找 Google Test
find_package(GTest REQUIRED)
enable_testing()
# 包含 osadaptor 头文件路径
include_directories(${PROJECT_SOURCE_DIR}/netflow/OSAdaptor/include/)
# 添加测试目标
add_executable(CGroupV2Test ./CGroupV2Test.cpp)
# 链接 Google Test 库
target_link_libraries(CGroupV2Test
        GTest::GTest
        GTest::Main
        pthread
        osadaptor
)
add_executable(ProcessTest ./ProcessTest.cpp)
# 链接 Google Test 库
target_link_libraries(ProcessTest
        GTest::GTest
        GTest::Main
        pthread
        osadaptor
)

# install
set(OSA_TESTS_LISTS CGroupV2Test
                    ProcessTest
)
set(OSA_TESTS_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/tests/osadaptor/)
install(TARGETS ${OSA_TESTS_LISTS}
        RUNTIME DESTINATION ${OSA_TESTS_INSTALL_DIR})