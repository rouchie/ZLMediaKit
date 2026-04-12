# ImGui 字体工具配置
# imgui_font 是独立工具，需要隔离项目的自定义 assert.h
add_executable(imgui_font imgui/misc/fonts/binary_to_compressed_c.cpp)

# 移除全局 include 路径，避免包含项目的 3rdpart/assert.h
set_target_properties(imgui_font PROPERTIES
    INCLUDE_DIRECTORIES ""
)

# 只添加必要的系统头文件路径
target_include_directories(imgui_font PRIVATE
    ${CMAKE_SYSTEM_INCLUDE_PATH}
)
