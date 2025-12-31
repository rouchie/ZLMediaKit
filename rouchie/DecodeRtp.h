#pragma once

#include "Base/RQModuleBase.h"

#include "Common/MediaSink.h"
#include "Rtsp/RtspDemuxer.h"
#include "Rtsp/RtpReceiver.h"

using namespace toolkit;
using namespace mediakit;

class FrameWriterImp
    : public FrameWriterInterface
    , public std::enable_shared_from_this<FrameWriterImp> {
public:
    using Ptr = std::shared_ptr<FrameWriterImp>;

public:
    // FrameWriterInterface 回调
    bool inputFrame(const Frame::Ptr &frame) override;
};

class RtpDecodeMod : public RQModuleBase, public RtpReceiver, private TrackListener
{
public:
    RtpDecodeMod();
    ~RtpDecodeMod();

public:
    // RQModuleBase 回调
    int OnStart() override;

public:
    // RtpReceiver 回调
    void onRtpSorted(RtpPacket::Ptr rtp, int index) override;

public:
    // TrackListener 回调
    bool addTrack(const Track::Ptr & track) override;
    void addTrackCompleted() override;
    void resetTracks() override;

private:
    RtspDemuxer::Ptr _demuxer;
    FrameWriterImp::Ptr _frame_writer;
};

