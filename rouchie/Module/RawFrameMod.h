#pragma once

#include "Base/RQModuleHelper.h"
#include "Record/MP4Demuxer.h"

using namespace toolkit;
using namespace mediakit;

class RawFrameMod : public RQModuleHelper<RawFrameMod> {
public:
    explicit RawFrameMod(std::string mp4File);

    int OnStart() override;

private:
    int OnHeartbeat(const RQMsg::Ptr &msg);
    int OnSubscriptionStream(const RQMsg::Ptr &msg);

private:
    uint32_t getCurrentStamp() const;
    void setCurrentStamp(uint32_t stamp);

private:
    std::string _mp4File;
    MP4Demuxer::Ptr _demuxer;

    int64_t _timerHeartbeat = 0;

    uint32_t _last_dts = 0;
    toolkit::Ticker _seek_ticker;

    std::list<mod_t> _mod_list;

    mediakit::VideoTrack::Ptr _videoTrack;
    mediakit::AudioTrack::Ptr _audioTrack;
};
