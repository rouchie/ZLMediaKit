
set(DEMO imgui_demo)

# 添加可执行文件
add_executable(${DEMO}
    main.cpp
)

target_link_libraries(${DEMO} ${LIB_IMGUI} spdlog::spdlog)
