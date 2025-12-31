#include "RtspPlayerMod.h"
#include "Rtsp/RtspPlayerImp.h"

#include "fmt/format.h"

RtspPlayerMod::RtspPlayerMod(const std::string& url)
    : RQModuleBase(MOD_RTSPPLAYER),  _url(url)
{
}

int RtspPlayerMod::OnStart()
{
    Bind(CMD_HEARTBEAT, &RtspPlayerMod::OnHeartbeat);
    Timer(1000, MSG(ID(), CMD_HEARTBEAT, 0), true);

    MediaTuple tuple;
    tuple.app = "live";
    tuple.stream = "stream1";

    ProtocolOption option;
    option.enable_mp4 = false;
    option.enable_hls = false;
    option.enable_hls_fmp4 = false;
    option.max_track = 16;

    _proxy = std::make_shared<PlayerProxy>(tuple, option, -1);
    _proxy->play(_url);

    return 0;
}

int RtspPlayerMod::OnHeartbeat(const RQMsg::Ptr& msg)
{
    return 0;
}

