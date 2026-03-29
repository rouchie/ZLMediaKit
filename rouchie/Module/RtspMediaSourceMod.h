#pragma once

#include "Base/RQModuleBase.h"
#include "Record/MP4Demuxer.h"
#include "Common/MultiMediaSourceMuxer.h"
#include "Muxer/RQRtspMediaSourceMuxer.h"

using namespace toolkit;
using namespace mediakit;

class RtspMediaSourceMod : public RQModuleHelper<RtspMediaSourceMod>, public mediakit::MediaSink {
public:
    RtspMediaSourceMod(const std::string& mp4File, const std::string& app, const std::string& stream);
    int OnStart() override;

protected:
    /// MediaSink
    bool onTrackReady(const Track::Ptr & track) override;
    void onAllTrackReady() override;
    bool onTrackFrame(const Frame::Ptr &frame) override;

private:
    int OnHeartbeat(const RQMsg::Ptr &msg);

private:
    void OpenSource();

    uint32_t getCurrentStamp() const;
    void setCurrentStamp(uint32_t stamp);

private:
    std::string _mp4File;
    std::string _app;
    std::string _stream;
    std::unordered_map<int, mediakit::Stamp> _stamps;

    MP4Demuxer::Ptr _demuxer;
    // RtspMediaSourceMuxer::Ptr _muxer;
    RQRtspMediaSourceMuxer::Ptr _muxer;

    int64_t _timerHeartbeat = 0;

    uint32_t _last_dts = 0;
    toolkit::Ticker _seek_ticker;
};
