#pragma once

#include "Base/RQModuleBase.h"
#include "Extension/CommonRtmp.h"

class TrackMod : public RQModuleHelper<TrackMod> {
public:
    TrackMod();
    int OnStart() override;

private:
    int OnRawData(const RQMsg::Ptr &msg);

private:
    uint32_t _nFrameCount = 0;
    bool _bTrackReady[2] = {false, false};
};
