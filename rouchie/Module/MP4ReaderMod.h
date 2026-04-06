#pragma once

#include "Base/RQModuleHelper.h"
#include "Record/MP4Demuxer.h"
#include "Common/MultiMediaSourceMuxer.h"

using namespace toolkit;
using namespace mediakit;

class MP4ReaderMod : public RQModuleHelper<MP4ReaderMod> {
public:
    MP4ReaderMod(const std::string& mp4File, const std::string& app, const std::string& stream);
    int OnStart() override;

private:
    int OnHeartbeat(const RQMsg::Ptr &msg);

private:
    uint32_t getCurrentStamp() const;
    void setCurrentStamp(uint32_t stamp);

private:
    std::string _mp4File;
    std::string _app;
    std::string _stream;

    MP4Demuxer::Ptr _demuxer;
    MultiMediaSourceMuxer::Ptr _muxer;

    int64_t _timerHeartbeat = 0;

    uint32_t _last_dts = 0;
    toolkit::Ticker _seek_ticker;
};
