#pragma once

#include "Base/RQModuleBase.h"
#include "Rtsp/RtspPlayer.h"
#include "Player/PlayerProxy.h"

using namespace toolkit;
using namespace mediakit;

class RtspPlayerMod : public RQModuleBase {
public:
    RtspPlayerMod(const std::string& url);
    int OnStart() override;

private:
    int OnHeartbeat(const RQMsg::Ptr &msg);

private:
    std::string _url;
    PlayerProxy::Ptr _proxy;
};
