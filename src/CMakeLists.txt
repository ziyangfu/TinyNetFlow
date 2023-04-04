find_package(Boost REQUIRED)
find_package(Protobuf)
find_package(CURL)
find_package(ZLIB)

add_subdirectory(./base)
add_subdirectory(./net)