# 主应用程序配置
find_package(libuv CONFIG REQUIRED)

add_executable(uv_test uv/main.cpp)
target_link_libraries(uv_test PRIVATE $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>)