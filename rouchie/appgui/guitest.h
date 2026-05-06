#pragma once

#include "guibase.h"

class GuiTest : public GuiBase {
public:
    GuiTest() = default;
    void operator()();

private:
    bool _topWindow = true;
    bool _init = false;
};

inline void GuiTest::operator()() {
    ImGui::Begin("Test");

    if (ImGui::CollapsingHeader("Source Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
        // mutliSource 的内容
        static std::string localFile;
        ImGui::Text("file"); ImGui::SameLine();
        ImGui::InputText("##localfile", &localFile);
        // ...
    }

    if (ImGui::CollapsingHeader("Output Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        static std::string outputPath;
        ImGui::Text("Output Path:");
        ImGui::InputText("##output", &outputPath);
    }

    if (ImGui::CollapsingHeader("Status")) {
        ImGui::Text("Current Status: Running");
    }

    ImGui::End();

    ImGui::Begin("Main Window");

    {
        if (!_init) {
            _init = TopWindow(_topWindow);
        }
        if (ImGui::Checkbox(u8"置顶##Topmost Window", &_topWindow)) {
            TopWindow(_topWindow);
        }
    }

    if (ImGui::BeginTabBar("MyTabBar")) {
        if (ImGui::BeginTabItem("Source")) {
            // 原来的 mutliSource 内容
            ImGui::Text("MP4 Source Configuration");
            // ... 其他控件
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Output")) {
            ImGui::Text("Output Settings");
            // ... 输出配置
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("About")) {
            ImGui::Text("About this tool");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
