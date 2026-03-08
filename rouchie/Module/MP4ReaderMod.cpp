#include "MP4ReaderMod.h"

MP4ReaderMod::MP4ReaderMod(const std::string& mp4File, const std::string& app, const std::string& stream)
    : RQModuleHelper<MP4ReaderMod>(MOD_MP4READER),  _mp4File(mp4File), _app(app), _stream(stream)
{
}

int MP4ReaderMod::OnStart()
{
    Bind(CMD_HEARTBEAT, &MP4ReaderMod::OnHeartbeat);

    _demuxer = std::make_shared<MP4Demuxer>();
    _demuxer->openMP4("test.mp4");

    ProtocolOption option;
    // 读取mp4文件并流化时，不重复生成mp4/hls文件  [AUTO-TRANSLATED:5d414546]
    // Read mp4 file and stream it, do not regenerate mp4/hls file repeatedly
    option.enable_mp4 = false;
    option.enable_hls = false;
    option.enable_hls_fmp4 = false;
    option.max_track = 16;

    MediaTuple tuple;
    tuple.app = _app;
    tuple.stream = _stream;

    // _muxer = std::make_shared<MultiMediaSourceMuxer>(tuple, _demuxer->getDurationMS() / 1000.0f, option);
    _muxer = std::make_shared<MultiMediaSourceMuxer>(tuple, 0.f, option);

    int interval = 10;

    auto tracks = _demuxer->getTracks(false);
    for (auto &track : tracks) {
        _muxer->addTrack(track);
        if (track->getTrackType() == TrackVideo) {
            auto vTrack = std::dynamic_pointer_cast<VideoTrack>(track);
            interval = 1000 / vTrack->getVideoFps();
        }
    }
    // 添加完毕所有track，防止单track情况下最大等待3秒  [AUTO-TRANSLATED:445e3403]
    // After all tracks are added, prevent the maximum waiting time of 3 seconds in the case of a single track
    _muxer->addTrackCompleted();

    _timerHeartbeat = Timer(interval, RQMsg::Build(ID(), CMD_HEARTBEAT));

    return 0;
}

int MP4ReaderMod::OnHeartbeat(const RQMsg::Ptr& msg)
{
    bool keyFrame;
    bool eof;

    do {
		if (_last_dts > getCurrentStamp() + 100) {
			return 0;
		}

        auto frame = _demuxer->readFrame(keyFrame, eof);
        if (!frame || eof) {
            InfoL << "MP4ReaderMod reached end of file.";
            setCurrentStamp(0);
            continue;
        }

        if (frame->keyFrame()) {
            // InfoL << fmt::format("dts({}) pts({}) keyFrame({}) codec({})", frame->dts(), frame->pts(), frame->keyFrame(), frame->getCodecName());
        }

        _last_dts = frame->dts();

        if (_muxer) {
            _muxer->inputFrame(frame);
        }
    } while (1);

    return 0;
}

uint32_t MP4ReaderMod::getCurrentStamp()
{
    return _seek_ticker.elapsedTime();
}

void MP4ReaderMod::setCurrentStamp(uint32_t stamp)
{
    _last_dts = stamp;
    _demuxer->seekTo(stamp);
    _seek_ticker.resetTime();
    _muxer->setTimeStamp(stamp);
}
