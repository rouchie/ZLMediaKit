#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <functional>
#include <initializer_list>
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

inline bool TopWindow(bool isTopMost) {
    // 获取当前 ImGui 窗口对应的底层平台窗口句柄 (HWND)
    ImGuiViewport *viewport = ImGui::GetWindowViewport();

#ifdef _WIN32
    // 🔥 修复点：优先使用 PlatformHandleRaw，如果为空则回退到 PlatformHandle
    HWND hwnd = viewport->PlatformHandleRaw ? (HWND)viewport->PlatformHandleRaw : (HWND)viewport->PlatformHandle;

    // 确保成功获取到了句柄
    if (hwnd) {
        // 根据复选框状态，设置或取消置顶
        ::SetWindowPos(hwnd, isTopMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return true;
    }
#endif
    return false;
}
