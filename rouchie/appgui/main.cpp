#include "guimediasource.h"
#include "guitest.h"

int main(int, char**)
{
    bool run = true;

    auto f = [&run]() {
        ImGui::Begin("Stop Window", nullptr, ImGuiWindowFlags_NoTitleBar);
        if (ImGui::Button("退出", ImVec2(200, 100))) {
            run = false;
        }
        ImGui::End();
    };

    const std::initializer_list<std::function<void()>> guiList = {
        f,
        GuiTest(),
        GuiMediaSource(),
    };

    return RunImgui(run, guiList);
}
