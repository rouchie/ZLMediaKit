cmake_minimum_required(VERSION 3.29)
project(Rouchie)

# 查找依赖
find_package(spdlog REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

#############################################################
# 包含各个模块配置
include(cmake/imgui.cmake)
include(cmake/imgui_font.cmake)
include(cmake/rouchie_lib.cmake)
include(cmake/app.cmake)
include(cmake/subdirs.cmake)
include(cmake/uv.cmake)
