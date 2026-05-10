#pragma once

#include "../guibase.h"

class Target {
public:
    using Ptr = std::shared_ptr<Target>;
public:
    virtual ~Target() = default;
    virtual std::string Request() = 0;
};

class Adaptee {
public:
    virtual ~Adaptee() = default;
    virtual std::string SpecificRequest() { return "Specific Request"; }
};

// 类适配器，c++特有，多重继承
class Adapter
    : public Target
    , public Adaptee {
public:
    std::string Request() override { return u8"多重继承:" + SpecificRequest(); }
};

// 对象适配器, 更推荐，组合优于继承
class Adapter2
    : public Target {
public:
    Adapter2() {}
    std::string Request() override { return u8"组合:" + _adaptee.SpecificRequest(); }

private:
    Adaptee _adaptee;
};

class GuiAdapter : public GuiBase {
public:
    GuiAdapter();
    void operator()() override;

private:
    std::string _text;

    Target::Ptr _target;
    Target::Ptr _target2;
};

inline GuiAdapter::GuiAdapter() {
    _target = std::make_shared<Adapter>();
    _target2 = std::make_shared<Adapter2>();
}

inline void GuiAdapter::operator()() {
    ImGui::Begin("适配器");

    if (ImGui::Button(u8"类适配器")) {
        _text = _target->Request();
    }

    if (ImGui::Button(u8"对象适配器")) {
        _text = _target2->Request();
    }

    ImGui::Text("%s", _text.c_str());

    ImGui::End();
}
