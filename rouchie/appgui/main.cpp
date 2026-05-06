#include "runimgui.h"
#include "guimain.h"

int main(int, char **) {
    bool run = true;
    const std::initializer_list<std::function<void()>> guiList = { GuiMain(run) };
    return RunImgui(run, guiList);
}
