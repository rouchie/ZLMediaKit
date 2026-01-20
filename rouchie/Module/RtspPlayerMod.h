#pragma once

#include "Base/RQModuleBase.h"
#include "Player/MediaPlayer.h"
#include <functional>
#include <string>
#include <vector>

using namespace toolkit;
using namespace mediakit;

class RtspPlayerMod
    : public RQModuleHelper<RtspPlayerMod>
    , public FrameWriterInterface {
public:
    RtspPlayerMod(const std::string &url, std::function<void(void *, const std::vector<std::string> &)> func, void *arg);
    int OnStart() override;

public:
    bool inputFrame(const Frame::Ptr &frame) override;

private:
    int OnHeartbeat(const RQMsg::Ptr &msg);

private:
    std::string _url;
    MediaPlayer::Ptr _player;

    std::function<void(void *, const std::vector<std::string> &)> _func;
    void *_arg;
};