#pragma once

#include "topwindow.h"
#include "imgui_stdlib.h"

#include <fmt/format.h>
#include <string>
#include <memory>

class GuiBase {
public:
    using Ptr = std::shared_ptr<GuiBase>;

public:
    GuiBase() = default;
    virtual ~GuiBase() = default;

    virtual void operator()() = 0;
};
