#pragma once

#include "../guibase.h"

class Color {
public:
    using Ptr = std::shared_ptr<Color>;
public:
    virtual ~Color() = default;
    virtual ImU32 getColor() = 0;
};

class White : public Color {
public:
    ImU32 getColor() override {
        return IM_COL32(255, 255, 255, 255);
    }
};

class Red : public Color {
public:
    ImU32 getColor() override {
        return IM_COL32(255, 0, 0, 255);
    }
};

class Sharp {
public:
    using Ptr = std::shared_ptr<Sharp>;
public:
    explicit Sharp(const Color::Ptr &color) : _color(color) {}
    virtual ~Sharp() = default;

    virtual void draw(const ImVec2 pos, const int width) = 0;

protected:
    Color::Ptr _color;
};

class Rect : public Sharp {
public:
    explicit Rect(const Color::Ptr &color) : Sharp(color) {}
    void draw(const ImVec2 pos, const int width) override {
        const auto windowDrawList = ImGui::GetWindowDrawList();
        const auto min = ImVec2(pos.x, pos.y);
        const auto max = ImVec2(pos.x + width, pos.y + width);
        windowDrawList->AddRect(min, max, _color->getColor(), 2.0f); // 10.0f 为圆角半径
    }
};

class Circle : public Sharp {
public:
    explicit Circle(const Color::Ptr &color) : Sharp(color) {}
    void draw(const ImVec2 pos, const int width) override {
        const auto windowDrawList = ImGui::GetWindowDrawList();
        const auto center = ImVec2(pos.x + width / 2, pos.y + width / 2);
        windowDrawList->AddCircle(center, width / 2, _color->getColor());
    }
};

class GuiBridge : public GuiBase {
public:
    GuiBridge();
    void operator()() override;

private:
    Sharp::Ptr _sharp;
};

inline GuiBridge::GuiBridge() {
    _sharp = std::make_unique<Rect>(std::make_unique<Red>());
}

inline void GuiBridge::operator()() {
    ImGui::Begin("桥接");

    if (ImGui::Button(u8"方红")) {
        _sharp = std::make_shared<Rect>(std::make_shared<Red>());
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"方白")) {
        _sharp = std::make_shared<Rect>(std::make_shared<White>());
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"圆红")) {
        _sharp = std::make_shared<Circle>(std::make_shared<Red>());
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"圆白")) {
        _sharp = std::make_shared<Circle>(std::make_shared<White>());
    }

    _sharp->draw(ImGui::GetCursorScreenPos(), 200.f);

    ImGui::End();
}
