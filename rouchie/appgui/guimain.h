#pragma once

#include "guibase.h"

#include "guimediasource.h"
#include "guitest.h"
#include "guiaac.h"

#include "design_patterns/guifactory.h"
#include "design_patterns/guiabstractfactory.h"

class GuiMain : public GuiBase {
public:
    GuiMain(bool& run);

    void operator()() override;

private:
    bool& _bRun;

    bool _bShowMediaSource = false;
    bool _bShowAAC = false;
    bool _bShowTest = false;

    Ptr _guiMediaSource;
    Ptr _guiAAC;
    Ptr _guiTest;

    bool _bShowFactory = false; // 工厂方法
    Ptr _guiFactory;

    bool _bShowAbstractFactory = false; // 抽象工厂方法
    Ptr _guiAbstractFactory;
};

inline GuiMain::GuiMain(bool &run) : _bRun(run) { }

#define CHECKBOX(NAME, VALUE)                                                                                                                    \
    ImGui::Checkbox(NAME, &_bShow##VALUE);                                                                                                                             \
    if (_bShow##VALUE) {                                                                                                                                               \
        if (!_gui##VALUE)                                                                                                                                              \
            _gui##VALUE = std::make_shared<Gui##VALUE>();                                                                                                                 \
        (*_gui##VALUE)();                                                                                                                                              \
    }

inline void GuiMain::operator()() {
    ImGui::Begin("MAIN WINDOW");

    if (ImGui::Button(u8"退出", ImVec2(-1.0f, 100))) {
        _bRun = false;
    }

    if (ImGui::CollapsingHeader("媒体功能##MediaServer", ImGuiTreeNodeFlags_AllowOverlap)) {
        ImGui::Checkbox(u8"MediaSource窗口", &_bShowMediaSource);
        ImGui::Checkbox(u8"AAC窗口", &_bShowAAC);
        ImGui::Checkbox(u8"TEST窗口", &_bShowTest);
    }

    if (ImGui::CollapsingHeader("设计模式##DesignPatterns", ImGuiTreeNodeFlags_AllowOverlap)) {
        ImGui::Text("创建型模式");

        CHECKBOX(u8"工厂方法", Factory);
        CHECKBOX(u8"抽象工厂方法", AbstractFactory);

        ImGui::Separator();
        ImGui::Text("结构型模式");
        ImGui::Separator();
        ImGui::Text("行为型模式");
        ImGui::Separator();
    }

    ImGui::End();

    if (_bShowMediaSource) {
        if (!_guiMediaSource) _guiMediaSource = std::make_shared<GuiMediaSource>();
        (*_guiMediaSource)();
    }

    if (_bShowAAC) {
        if (!_guiAAC) _guiAAC = std::make_shared<GuiAAC>();
        (*_guiAAC)();
    }

    if (_bShowTest) {
        if (!_guiTest) _guiTest = std::make_shared<GuiTest>();
        (*_guiTest)();
    }
}
