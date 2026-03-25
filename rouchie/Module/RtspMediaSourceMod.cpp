#include "RtspMediaSourceMod.h"

#include "Rtsp/RtspMediaSourceMuxer.h"

RtspMediaSourceMod::RtspMediaSourceMod(const std::string &mp4File, const std::string &app, const std::string &stream)
    : RQModuleHelper<RtspMediaSourceMod>(MOD_MP4READER)
    , _mp4File(mp4File)
    , _app(app)
    , _stream(stream) { }

int RtspMediaSourceMod::OnStart() {
    Bind(CMD_HEARTBEAT, &RtspMediaSourceMod::OnHeartbeat);

    _demuxer = std::make_shared<MP4Demuxer>();
    _demuxer->openMP4(_mp4File);

    OpenSource();

    return 0;
}

bool RtspMediaSourceMod::onTrackReady(const Track::Ptr &track) {
    auto &stamp = _stamps[track->getIndex()];
    _muxer->addTrack(track);
    return true;
}

void RtspMediaSourceMod::onAllTrackReady() {
    _muxer->addTrackCompleted();

    mediakit::Stamp *first = nullptr;
    for (auto &pr : _stamps) {
        if (!first) {
            first = &pr.second;
        } else {
            pr.second.syncTo(*first);
        }
    }
}

bool RtspMediaSourceMod::onTrackFrame(const Frame::Ptr &frame_in) {
    auto frame = frame_in;
    frame = std::make_shared<FrameStamp>(frame, _stamps[frame->getIndex()], 2);
    if (_muxer) {
        _muxer->inputFrame(frame);
    }
    return true;
}

int RtspMediaSourceMod::OnHeartbeat(const RQMsg::Ptr &msg) {
    bool keyFrame;
    bool eof;

    do {
        if (_last_dts > getCurrentStamp() + 100) {
            return 0;
        }

        auto frame = _demuxer->readFrame(keyFrame, eof);
        if (!frame || eof) {
            setCurrentStamp(0);
            continue;
        }

        _last_dts = frame->dts();

        this->inputFrame(frame);
    } while (true);

    return 0;
}

void RtspMediaSourceMod::OpenSource() {
    ProtocolOption option;

    MediaTuple tuple;
    tuple.app = _app;
    tuple.stream = _stream;

    _muxer = std::make_shared<RtspMediaSourceMuxer>(tuple, option, std::make_shared<TitleSdp>(0));

    uint64_t interval = 10;

    auto tracks = _demuxer->getTracks(false);
    for (auto &track : tracks) {
        InfoL << fmt::format("type[{}] ready[{}]", track->getTrackTypeStr(), track->ready());

        this->addTrack(track);

        if (track->getTrackType() == TrackVideo) {
            auto vTrack = std::dynamic_pointer_cast<VideoTrack>(track);
            interval = static_cast<uint64_t>(1000 / vTrack->getVideoFps());
        }
    }

    this->addTrackCompleted();

    _timerHeartbeat = Timer(interval, RQMsg::Build(ID(), CMD_HEARTBEAT));
}

uint32_t RtspMediaSourceMod::getCurrentStamp() const {
    return _seek_ticker.elapsedTime();
}

void RtspMediaSourceMod::setCurrentStamp(uint32_t stamp) {
    _last_dts = stamp;
    _demuxer->seekTo(stamp);
    _seek_ticker.resetTime();
}
