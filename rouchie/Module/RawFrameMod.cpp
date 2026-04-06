#include "RawFrameMod.h"

#include "Base/RQCore.h"
#include "Msg/RQFrameMsg.h"
#include "Msg/RQTrackMsg.h"

#include "Extension/Factory.h"

RawFrameMod::RawFrameMod(std::string mp4File)
    : RQModuleHelper<RawFrameMod>(MOD_RAW_FRAME)
    , _mp4File(std::move(mp4File)) { }

int RawFrameMod::OnStart() {
    Bind(CMD_HEARTBEAT, &RawFrameMod::OnHeartbeat);
    Bind(CMD_SUBSCRIPTION_STREAM, &RawFrameMod::OnSubscriptionStream);

    _demuxer = std::make_shared<MP4Demuxer>();
    _demuxer->openMP4(_mp4File);

    int interval = 10;

    const auto tracks = _demuxer->getTracks(false);
    for (auto &track : tracks) {
        const auto extra = track->getExtraData();

        if (track->getTrackType() == TrackVideo) {
            const auto vTrack = std::dynamic_pointer_cast<VideoTrack>(track);
            interval = static_cast<int>(1000 / vTrack->getVideoFps());
            _videoTrack = std::dynamic_pointer_cast<VideoTrack>(track->clone());
        } else if (track->getTrackType() == TrackAudio) {
            // _audioTrack = std::dynamic_pointer_cast<AudioTrack>(Factory::getTrackByAbstractTrack(track));
            // _audioTrack->setExtraData(reinterpret_cast<const uint8_t *>(extra->data()), extra->size());
            _audioTrack = std::dynamic_pointer_cast<AudioTrack>(track->clone());
        }
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
            const auto frameMsg = std::make_shared<RQFrameMsg>(ID(), recver, CMD_STREAM_FRAME_DATA);
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

int RawFrameMod::OnSubscriptionStream(const RQMsg::Ptr &msg) {
    // 发送轨道信息
    const auto trackMsg = std::make_shared<RQTrackMsg>(CMD_STREAM_TRACK_INFO);
    trackMsg->videoTrack = _videoTrack;
    trackMsg->audioTrack = _audioTrack;

    SendMessage(msg->_sender, trackMsg);

    // 添加订阅者
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
