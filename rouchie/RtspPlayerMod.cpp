#include "RtspPlayerMod.h"
#include "Rtsp/RtspPlayerImp.h"

#include "fmt/format.h"

RtspPlayerMod::RtspPlayerMod(const std::string& url)
    : RQModuleHelper<RtspPlayerMod>(MOD_RTSPPLAYER),  _url(url)
{
}

int RtspPlayerMod::OnStart()
{
    Bind(CMD_HEARTBEAT, &RtspPlayerMod::OnHeartbeat);
    Bind(CMD_STOP, [this](const RQMsg::Ptr& msg) { 
        _player.reset();
        return 0;
        });

    Timer(1000, RQMsg::Build(ID(), CMD_HEARTBEAT, 0), true);

    auto weakSelf = std::weak_ptr<RQModuleBase>(shared_from_this());

    _player = std::make_shared<MediaPlayer>(this->GetPoller());

    _player->setOnPlayResult([this, weakSelf](const toolkit::SockException &ex) {
        auto strongSelf = weakSelf.lock();
        if (!strongSelf) {
            return;
        }

        InfoL << "PlayResult: " << ex.what();

        if (!ex) {
            auto tracks = _player->getTracks();
            for (auto &track : tracks) {
                InfoL << fmt::format("Track codec: {}", track->getCodecName());
                track->addDelegate(shared_from_this());
            }
        }
    });

    _player->setOnShutdown([](const toolkit::SockException &ex) {
        InfoL << "Shutdown: " << ex.what();
        });

    _player->setOnResume([]() {
        InfoL << "Resume playback";
        });

    _player->play(_url);

    SendDelayMsg(1000*20, RQMsg::Build(ID(), CMD_STOP));

    return 0;

    MediaTuple tuple;
    tuple.app = "live";
    tuple.stream = "stream1";

    ProtocolOption option;
    option.enable_mp4 = false;
    option.enable_hls = false;
    option.enable_hls_fmp4 = false;
    option.max_track = 16;

    _proxy = std::make_shared<PlayerProxy>(tuple, option, -1);

    _proxy->setPlayCallbackOnce([](const toolkit::SockException &ex) {
        InfoL << ex.what();
    });

    _proxy->setOnClose([](const toolkit::SockException &ex) {
        InfoL << "OnClose " << ex.what();
    });

    _proxy->setOnDisconnect([]() {
        InfoL << "OnDisconnect" << "Disconnected from server";
    });

    _proxy->setOnConnect([](const TranslationInfo & info) {
        InfoL << "OnConnect" <<  info.start_time_stamp;
    });

    _proxy->play(_url);

    return 0;
}

bool RtspPlayerMod::inputFrame(const Frame::Ptr& frame)
{
    auto frameinfo = [frame]() { return fmt::format("dts({}) pts({}) keyFrame({}) codec({})", frame->dts(), frame->pts(), frame->keyFrame(), frame->getCodecName()); }();
    InfoL << frameinfo;
    return true;
}

int RtspPlayerMod::OnHeartbeat(const RQMsg::Ptr& msg)
{
    return 0;
}
