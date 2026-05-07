#pragma once

#include "../guibase.h"

class Tool {
public:
    static Tool& getInstance();

private:
    Tool() = default;
    ~Tool() = default;
    Tool(const Tool&) = delete;
    Tool& operator=(const Tool&) = delete;

public:
    std::string hello();
    std::string world();

private:
    int _nHelloCnt = 0;
    int _nWorldCnt = 0;
};

// c++11 及之后，最好的单例写法
inline Tool &Tool::getInstance() {
    static Tool instance;
    return instance;
}

inline std::string Tool::hello() {
    return fmt::format("hello: {}", ++_nHelloCnt);
}

inline std::string Tool::world() {
    return fmt::format("world: {}", ++_nWorldCnt);
}

class GuiSingleton : public GuiBase {
public:
    GuiSingleton();
    void operator()() override;

private:
    int _nSelected = 0;   // 0: 选项A, 1: 选项B, 2: 选项C
    std::string _text;
};

inline GuiSingleton::GuiSingleton() {}

inline void GuiSingleton::operator()() {
    ImGui::Begin("单例");

    const int nSelected = _nSelected;

    if (ImGui::Button(u8"Hello", ImVec2(-1.0f, 0))) {
        _text = Tool::getInstance().hello();
    }
    if (ImGui::Button(u8"World", ImVec2(-1.0f, 0))) {
        _text = Tool::getInstance().world();
    }

    ImGui::Text("%s", _text.c_str());

    ImGui::End();
}
