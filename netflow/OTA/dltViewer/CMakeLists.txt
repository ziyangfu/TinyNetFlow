# dlt_viewer的编译安装配置，源代码在third_party中，编译完成的Linux-X86-64平台可执行文件在tools下
if (${ADD_DLT_VIEWER_EXEC})
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/tools/DLTViewer/
            DESTINATION ${CMAKE_INSTALL_PREFIX}/tools/DLTViewer)
endif ()
