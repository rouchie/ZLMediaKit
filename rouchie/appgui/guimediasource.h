#pragma once

#include "../imguidemo/runimgui.h"
#include "imgui_stdlib.h"

#include "Module/MP4ReaderMod.h"
#include "Base/RQCoreCreateModule.h"

#include <fmt/format.h>
#include <string>

class GuiMediaSource {
public:
    GuiMediaSource() = default;
    void operator()();

private:
    void StatusChange();

private:
    std::string _localFile = "test.mp4";
    std::string _app = "live";
    std::string _stream = "stream";

    std::string _btnName = u8"起流";

    MP4ReaderMod::Ptr _mp4ReadMod;
};

inline void GuiMediaSource::operator()() {
    ImGui::Begin("Mutli Source");

    const float width = ImGui::CalcTextSize("stream").x + 5;
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, width);  // 第一列宽度固定

    auto f = [](const char* text, std::string& value) {
        ImGui::Text(text);
        ImGui::NextColumn();
        ImGui::InputText(fmt::format("##{}", text).c_str(), &value);
        ImGui::NextColumn();  // 移到下一行的第一列
    };

    f("file", _localFile);
    f("app", _app);
    f("stream", _stream);

    ImGui::Columns(1);  // 恢复单列

    if (ImGui::Button(_btnName.c_str())) {
        StatusChange();
    }
    ImGui::End();
}

inline void GuiMediaSource::StatusChange() {
    if (_mp4ReadMod) {
        _btnName = u8"起流";
        _mp4ReadMod.reset();
    } else {
        _btnName = u8"停流";
        _mp4ReadMod = CreateModule<MP4ReaderMod>(_localFile, _app, _stream);
    }
}
