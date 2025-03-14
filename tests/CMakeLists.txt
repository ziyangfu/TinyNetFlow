# netflow tests top CMakeLists.txt
if (${BUILD_OSA_TESTS})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/osadaptor)
endif ()

# install test running script
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/run_all_tests.sh
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ
        DESTINATION ${CMAKE_INSTALL_PREFIX}/tests)