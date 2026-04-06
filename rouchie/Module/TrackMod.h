#pragma once

#include "Base/RQModuleHelper.h"
#include "Common/MediaSink.h"
#include "Extension/CommonRtmp.h"

class TrackMod : public RQModuleHelper<TrackMod>, public mediakit::MediaSink {
public:
    TrackMod();
    int OnStart() override;

private:
    int OnTrackInfo(const RQMsg::Ptr &msg);
    int OnRawData(const RQMsg::Ptr &msg);

protected:
    /// MediaSink
    bool onTrackReady(const mediakit::Track::Ptr & track) override;
    void onAllTrackReady() override;
    bool onTrackFrame(const mediakit::Frame::Ptr &frame) override;

private:
    uint32_t _nFrameCount = 0;
    bool _bTrackReady[2] = {false, false};
};
