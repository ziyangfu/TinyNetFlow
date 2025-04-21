# osadaptor-arxml单元测试集合
message(STATUS "\t will compile arxml unit tests")
# 查找 Google Test
find_package(GTest REQUIRED)
enable_testing()
# 包含 libarxml 头文件路径
include_directories(${PROJECT_SOURCE_DIR}/netflow/xmlParse/include/)
# 添加测试目标
add_executable(ArxmlUintTest
                    ./arxml_node_test.cpp
                    ./arxml_reader_test.cpp
)
# 链接 Google Test 库
target_link_libraries(ArxmlUintTest
        GTest::GTest
        GTest::Main
        pthread
        arxml
)

# install
set(ARXML_TESTS_LISTS
            ArxmlUintTest
)
set(ARXML_TESTS_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/tests/xmlParse/)
install(TARGETS ${ARXML_TESTS_LISTS}
        RUNTIME DESTINATION ${ARXML_TESTS_INSTALL_DIR})