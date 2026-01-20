#include "RtspPlayerMod.h"
#include "Base/RQCore.h"
#include "Util/onceToken.h"
#include "fmt/format.h"
#include <functional>
#include <mutex>
#include <sal.h>
#include <string>
#include <vector>

RtspPlayerMod::RtspPlayerMod(const std::string &url, std::function<void(void *, const std::vector<std::string> &)> func, void *arg)
    : RQModuleHelper<RtspPlayerMod>(MOD_RTSPPLAYER)
    , _url(url)
    , _func(func)
    , _arg(arg) {}

int RtspPlayerMod::OnStart() {
    Bind(CMD_HEARTBEAT, &RtspPlayerMod::OnHeartbeat);
    Bind(CMD_STOP, [this](const RQMsg::Ptr &msg) {
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

    _player->setOnShutdown([](const toolkit::SockException &ex) { InfoL << "Shutdown: " << ex.what(); });

    _player->setOnResume([]() { InfoL << "Resume playback"; });

    _player->play(_url);

    SendDelayMsg(1000 * 20, RQMsg::Build(ID(), CMD_STOP));

    return 0;
}

bool RtspPlayerMod::inputFrame(const Frame::Ptr &frame) {
    if (frame->getTrackType() != TrackVideo)
        return true;

    std::vector<std::string> v(7);
    v[0] = fmt::format("{}", frame->dts());
    v[1] = fmt::format("{}", frame->pts());
    v[2] = fmt::format("{}", frame->keyFrame());
    v[3] = fmt::format("{}", frame->configFrame());
    v[4] = fmt::format("{}", frame->dropAble());
    v[5] = fmt::format("{}", frame->decodeAble());
    v[6] = fmt::format("{}", frame->getCodecName());

    _func(_arg, v);

    return true;
}

int RtspPlayerMod::OnHeartbeat(const RQMsg::Ptr &msg) {
    return 0;
}

std::mutex g_mutex;
std::map<int, RQModuleBase::Ptr> g_players;
int id = 0;

int AddPlayer(const std::string &url, std::function<void(void *, const std::vector<std::string> &)> func, void *arg) {
    static onceToken token([]() {
        Logger::Instance().add(std::make_shared<ConsoleChannel>("ConsoleChannel"));
        Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());
    });

    RQModuleBase::Ptr player = CreateModule<RtspPlayerMod>(url, func, arg);

    std::lock_guard<std::mutex> lck(g_mutex);
    g_players[++id] = player;

    return id;
}

void DelPlayer(int id) {
    std::lock_guard<std::mutex> lck(g_mutex);
    g_players.erase(id);
}