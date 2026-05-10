#pragma once

#include "guibase.h"

#include "guimediasource.h"
#include "guitest.h"
#include "guiaac.h"

#include "design_patterns/guifactory.h"
#include "design_patterns/guiabstractfactory.h"
#include "design_patterns/guisingleton.h"
#include "design_patterns/guiadapter.h"
#include "design_patterns/guibridge.h"

class GuiMain : public GuiBase {
public:
    explicit GuiMain(bool& run);

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

    bool _bShowSingleton = false; // 单例
    Ptr _guiSingleton;

    bool _bShowAdapter = false; // 适配器
    Ptr _guiAdapter;

    bool _bShowBridge = false; // 桥接
    Ptr _guiBridge;
};

inline GuiMain::GuiMain(bool &run) : _bRun(run) { }

#define CHECKBOX(NAME, VALUE)                                                                                                                                  \
    ImGui::Checkbox(NAME, &_bShow##VALUE);                                                                                                                     \
    if (_bShow##VALUE) {                                                                                                                                       \
        if (!_gui##VALUE)                                                                                                                                      \
            _gui##VALUE = std::make_shared<Gui##VALUE>();                                                                                                      \
        (*_gui##VALUE)();                                                                                                                                      \
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
        CHECKBOX(u8"抽象工厂", AbstractFactory);
        CHECKBOX(u8"单例", Singleton);

        ImGui::Separator();
        ImGui::Text("结构型模式");

        CHECKBOX(u8"适配器", Adapter);
        CHECKBOX(u8"桥接", Bridge);

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
