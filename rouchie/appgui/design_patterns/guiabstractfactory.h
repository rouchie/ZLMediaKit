#pragma once

#include "../guibase.h"

// 抽象工厂方法
// 工厂方法模式和抽象工厂模式区别：
// 工厂方法：京东上买手机，有很多厂家可以选择
// 抽象工厂：京东上买手机，不仅有很多家，还有很多套餐可以选择，可以搭配充电器、手机壳、贴膜

class Phone {
public:
    using Ptr = std::shared_ptr<Phone>;
public:
    virtual ~Phone() = default;
    virtual std::string name() = 0;
};

class IPhone : public Phone {
public:
    std::string name() override { return "IPhone"; }
};

class VivoPhone : public Phone {
public:
    std::string name() override { return "VivoPhone"; }
};

class Charger {
public:
    using Ptr = std::shared_ptr<Charger>;
public:
    virtual ~Charger() = default;
    virtual bool Fast() = 0;
};

class SlowCharger : public Charger {
public:
    bool Fast() override { return false; }
};

class FastCharger : public Charger {
public:
    bool Fast() override { return true; }
};

class Bundle {
public:
    using Ptr = std::shared_ptr<Bundle>;
public:
    virtual ~Bundle() = default;
    virtual Phone::Ptr Phone() = 0;
    virtual Charger::Ptr Charger() = 0;
};

class ABundle : public Bundle {
public:
    Phone::Ptr Phone() override { return std::make_shared<IPhone>(); }
    Charger::Ptr Charger() override { return std::make_shared<SlowCharger>(); }
};

class BBundle : public Bundle {
public:
    Phone::Ptr Phone() override { return std::make_shared<VivoPhone>(); }
    Charger::Ptr Charger() override { return std::make_shared<FastCharger>(); }
};

class CBundle : public Bundle {
public:
    Phone::Ptr Phone() override { return std::make_shared<VivoPhone>(); }
    Charger::Ptr Charger() override { return std::make_shared<SlowCharger>(); }
};

class GuiAbstractFactory : public GuiBase {
public:
    GuiAbstractFactory();
    void operator()() override;

private:
    int _nSelected = 0;   // 0: 选项A, 1: 选项B, 2: 选项C
    Bundle::Ptr _bundle;
};

inline GuiAbstractFactory::GuiAbstractFactory() {
    _bundle = std::make_shared<ABundle>();
}

inline void GuiAbstractFactory::operator()() {
    ImGui::Begin("抽象工厂方法");

    const int nSelected = _nSelected;

    ImGui::RadioButton("套餐A", &_nSelected, 0);
    ImGui::SameLine();
    ImGui::RadioButton("套餐B", &_nSelected, 1);
    ImGui::SameLine();
    ImGui::RadioButton("套餐C", &_nSelected, 2);

    if (nSelected != _nSelected && _nSelected == 0) {
        _bundle = std::make_shared<ABundle>();
    } else if (nSelected != _nSelected && _nSelected == 1) {
        _bundle = std::make_shared<BBundle>();
    } else if (nSelected != _nSelected && _nSelected == 2) {
        _bundle = std::make_shared<CBundle>();
    }

    ImGui::Text("%s:%s", _bundle->Phone()->name().c_str(), _bundle->Charger()->Fast() ? "快充" : "慢充");

    ImGui::End();
}
