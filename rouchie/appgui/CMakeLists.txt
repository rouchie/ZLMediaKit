
set(DEMO appgui)

# 添加可执行文件
add_executable(${DEMO}
    main.cpp
)

message(${PROJECT_SOURCE_DIR}/../imguidemo)

target_include_directories(${DEMO} PRIVATE
    ${PROJECT_SOURCE_DIR}/imguidemo
)

target_link_libraries(${DEMO} ${PROJECT_NAME} ${LIB_IMGUI} spdlog::spdlog)
