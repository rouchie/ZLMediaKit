#include "runimgui.h"
#include <iostream>

bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void demo_window() {
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
}

void main_window() {
    static float f = 0.0f;
    static int counter = 0;

    const ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", reinterpret_cast<float *>(&clear_color)); // Edit 3 floats representing a color

    if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}

void another_window() {
    // 3. Show another simple window.
    if (show_another_window) {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
}

void test_window() {
    ImGui::Begin("Test Window");
    ImGui::Text("www.rouchie.com"); ImGui::SameLine();
    ImGui::Text("中文字体搞一些");
    if (ImGui::Button("啥子", ImVec2(200, 100))) {
    }

    static std::string name(128, 0);
    static std::string pwd(128, 0);
    ImGui::InputText("UserName", const_cast<char *>(name.data()), name.size());
    ImGui::InputText("PassWord", const_cast<char *>(pwd.data()), pwd.size());

    ImGui::End();
}

// Main code
int main(int, char**)
{
    bool run = true;
    auto f = [&run]() {
        ImGui::Begin("Stop Window", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        if (ImGui::Button("退出", ImVec2(200, 100))) {
            run = false;
        }
        ImGui::End();
    };
    return RunImgui(run, {f, demo_window, main_window, another_window, test_window});
}
