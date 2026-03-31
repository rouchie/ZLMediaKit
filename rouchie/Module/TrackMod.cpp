#include "TrackMod.h"

#include "Base/RQCore.h"
#include "Msg/RQFrameMsg.h"
#include "Extension/Factory.h"

TrackMod::TrackMod()
    : RQModuleHelper<TrackMod>(ModID()) { }

int TrackMod::OnStart() {
    Bind(CMD_FRAME_DATA, &TrackMod::OnRawData);

    const auto msg = RQMsg::Build(ID(), MOD_RAW_FRAME, CMD_SUBCP_FRAME, 0);
    SendMsg(msg);

    return 0;
}

int TrackMod::OnRawData(const RQMsg::Ptr &msg) {
    const auto frameMsg = std::dynamic_pointer_cast<RQFrameMsg>(msg);
    if (!frameMsg) {
        return 0;
    }

    // InfoL << fmt::format(
    //     "dts:{} pts:{} buffer:{} codec:{} track:{}", frameMsg->dts, frameMsg->pts, frameMsg->buffer->size(),
    //     mediakit::getCodecName(static_cast<mediakit::CodecId>(frameMsg->codecId)),
    //     mediakit::getTrackString(static_cast<mediakit::TrackType>(frameMsg->trackType)));

    _nFrameCount++;

    if (_bTrackReady[frameMsg->trackType] == false) {
        auto video = mediakit::Factory::getTrackByCodecId(frameMsg->codecId);
    }

    return 0;
}
