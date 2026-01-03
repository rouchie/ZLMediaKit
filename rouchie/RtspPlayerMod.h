#pragma once

#include "Base/RQModuleBase.h"
#include "Base/RQCore.h"
#include "Rtsp/RtspPlayer.h"
#include "Player/PlayerProxy.h"

using namespace toolkit;
using namespace mediakit;

class RtspPlayerMod : public RQModuleHelper<RtspPlayerMod>, public FrameWriterInterface {
public:
    RtspPlayerMod(const std::string& url);
    int OnStart() override;

public:
    bool inputFrame(const Frame::Ptr &frame) override;

private:
    int OnHeartbeat(const RQMsg::Ptr &msg);

private:
    std::string _url;
    PlayerProxy::Ptr _proxy;

    MediaPlayer::Ptr _player;
};
