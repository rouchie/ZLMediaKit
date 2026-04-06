#include "TrackMod.h"

#include "Base/RQCore.h"
#include "Extension/Factory.h"
#include "Msg/RQFrameMsg.h"
#include "Msg/RQTrackMsg.h"

TrackMod::TrackMod()
    : RQModuleHelper<TrackMod>(ModID()) { }

int TrackMod::OnStart() {
    Bind(CMD_STREAM_TRACK_INFO, &TrackMod::OnTrackInfo);
    Bind(CMD_STREAM_FRAME_DATA, &TrackMod::OnRawData);

    const auto msg = RQMsg::Build(ID(), MOD_RAW_FRAME, CMD_SUBSCRIPTION_STREAM, 0);
    SendMsg(msg);

    return 0;
}

int TrackMod::OnTrackInfo(const RQMsg::Ptr &msg) {
    const auto trackMsg = std::dynamic_pointer_cast<RQTrackMsg>(msg);
    if (!trackMsg) {
        return 0;
    }

    if (trackMsg->videoTrack) {
        this->addTrack(trackMsg->videoTrack);
    }

    if (trackMsg->audioTrack) {
        this->addTrack(trackMsg->audioTrack);
    }

    this->addTrackCompleted();

    return 0;
}

int TrackMod::OnRawData(const RQMsg::Ptr &msg) {
    const auto frameMsg = std::dynamic_pointer_cast<RQFrameMsg>(msg);
    if (!frameMsg) {
        return 0;
    }

    _nFrameCount++;

    // 帧数据 h264 的时候，没有sps、pps帧，所以track必须提前添加sps、pps等信息，不然没法使用帧数据
    // 帧数据 aac 的时候，没有adts头，所以track必须提前添加adts头信息，不然没法使用帧数据
    const auto frame = mediakit::Factory::getFrameFromBuffer(frameMsg->codecId, frameMsg->buffer, frameMsg->dts, frameMsg->pts);

    // InfoL << fmt::format(
    //     "dts:{} pts:{} buffer:{} codec:{} track:{} index:{}", frameMsg->dts, frameMsg->pts, frameMsg->buffer->size(),
    //     mediakit::getCodecName(static_cast<mediakit::CodecId>(frameMsg->codecId)),
    //     mediakit::getTrackString(static_cast<mediakit::TrackType>(frameMsg->trackType)), frame->getIndex());

    this->inputFrame(frame);

    return 0;
}

bool TrackMod::onTrackReady(const mediakit::Track::Ptr &track) {
    return MediaSink::onTrackReady(track);
}

void TrackMod::onAllTrackReady() {
    MediaSink::onAllTrackReady();
}

bool TrackMod::onTrackFrame(const mediakit::Frame::Ptr &frame) {
    return MediaSink::onTrackFrame(frame);
}
