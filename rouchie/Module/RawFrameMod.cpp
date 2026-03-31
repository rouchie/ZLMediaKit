#include "RawFrameMod.h"

#include "Base/RQCore.h"
#include "Msg/RQFrameMsg.h"

RawFrameMod::RawFrameMod(std::string mp4File)
    : RQModuleHelper<RawFrameMod>(MOD_RAW_FRAME)
    , _mp4File(std::move(mp4File)) { }

int RawFrameMod::OnStart() {
    Bind(CMD_HEARTBEAT, &RawFrameMod::OnHeartbeat);
    Bind(CMD_SUBCP_FRAME, &RawFrameMod::OnSubscriptionFrame);

    _demuxer = std::make_shared<MP4Demuxer>();
    _demuxer->openMP4(_mp4File);

    int interval = 10;

    const auto track = std::dynamic_pointer_cast<VideoTrack>(_demuxer->getTrack(TrackVideo));
    if (track) {
        interval = static_cast<int>(1000 / track->getVideoFps());
    }

    _timerHeartbeat = Timer(interval, RQMsg::Build(ID(), CMD_HEARTBEAT));

    return 0;
}

int RawFrameMod::OnHeartbeat(const RQMsg::Ptr &msg) {
    bool keyFrame;
    bool eof;

    do {
        if (_last_dts > getCurrentStamp() + 100) {
            return 0;
        }

        const auto frame = _demuxer->readFrame(keyFrame, eof);
        if (!frame || eof) {
            setCurrentStamp(0);
            continue;
        }

        for (auto &recver : _mod_list) {
            const auto frameMsg = std::make_shared<RQFrameMsg>(ID(), recver, CMD_FRAME_DATA);
            const auto buffer = toolkit::BufferRaw::create();
            buffer->assign(frame->data(), frame->size());

            frameMsg->codecId = frame->getCodecId();
            frameMsg->trackType = frame->getTrackType();
            frameMsg->dts = frame->dts();
            frameMsg->pts = frame->pts();
            frameMsg->buffer = buffer;

            SendMsg(frameMsg);
        }

        _last_dts = frame->dts();
    } while (true);

    return 0;
}

int RawFrameMod::OnSubscriptionFrame(const RQMsg::Ptr &msg) {
    _mod_list.push_back(msg->_sender);
    return 0;
}

uint32_t RawFrameMod::getCurrentStamp() const {
    return _seek_ticker.elapsedTime();
}

void RawFrameMod::setCurrentStamp(uint32_t stamp) {
    _last_dts = stamp;
    _demuxer->seekTo(stamp);
    _seek_ticker.resetTime();
}
