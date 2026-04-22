# ImGui 库配置
find_package(glfw3 CONFIG REQUIRED)

set(LIB_IMGUI Imgui)
set(IMGUI_SRC_LIST
	imgui/imconfig.h
	imgui/imgui.cpp
	imgui/imgui.h
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_internal.h
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/imstb_rectpack.h
	imgui/imstb_textedit.h
	imgui/imstb_truetype.h
	imgui/backends/imgui_impl_glfw.cpp
	imgui/backends/imgui_impl_glfw.h
	imgui/backends/imgui_impl_opengl3.cpp
	imgui/backends/imgui_impl_opengl3.h
	imgui/backends/imgui_impl_opengl3_loader.h
	imgui/misc/cpp/imgui_stdlib.cpp
	imgui/misc/cpp/imgui_stdlib.h
)

add_library(${LIB_IMGUI} STATIC ${IMGUI_SRC_LIST})

# 移除全局 include 路径，避免包含项目的 3rdpart/assert.h
set_target_properties(${LIB_IMGUI} PROPERTIES
	INCLUDE_DIRECTORIES ""
)

target_include_directories(${LIB_IMGUI} PUBLIC
	${CMAKE_SYSTEM_INCLUDE_PATH}
	${PROJECT_SOURCE_DIR}/imgui
	${PROJECT_SOURCE_DIR}/imgui/backends
	${PROJECT_SOURCE_DIR}/imgui/misc/cpp
)
target_link_libraries(${LIB_IMGUI} PUBLIC glfw opengl32)
