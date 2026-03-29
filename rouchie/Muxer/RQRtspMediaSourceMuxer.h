//
// Created by rouchie on 2026/3/29.
//

#ifndef ZLMEDIAKIT_RQRTSPMEDIASOURCEMUXER_H
#define ZLMEDIAKIT_RQRTSPMEDIASOURCEMUXER_H

#include "Rtsp/RtspMuxer.h"
#include "Rtsp/RtspMediaSource.h"

class RQRtspMediaSourceMuxer
    : public mediakit::RtspMuxer
    , public toolkit::RingDelegate<mediakit::RtpPacket::Ptr>
    , public mediakit::MediaSourceEventInterceptor
    , public std::enable_shared_from_this<RQRtspMediaSourceMuxer> {
public:
    using Ptr = std::shared_ptr<RQRtspMediaSourceMuxer>;

    RQRtspMediaSourceMuxer(const toolkit::EventPoller::Ptr &poller, const mediakit::MediaTuple& tuple, const mediakit::TitleSdp::Ptr &title = nullptr);

public:
    // RingDelegate override
    void onWrite(std::shared_ptr<mediakit::RtpPacket> in, bool is_key) override;

    // RtspMuxer override
    bool addTrack(const mediakit::Track::Ptr &track) override;
    void addTrackCompleted() override;
    bool inputFrame(const mediakit::Frame::Ptr &frame) override;

    // MediaSourceEvent override
    int totalReaderCount(mediakit::MediaSource &sender) override;
    void onReaderChanged(mediakit::MediaSource &sender, int size) override;
    toolkit::EventPoller::Ptr getOwnerPoller(mediakit::MediaSource &sender) override;

private:
    mediakit::RtspMediaSource::Ptr _media_src;
    toolkit::EventPoller::Ptr _poller;
};

#endif //ZLMEDIAKIT_RQRTSPMEDIASOURCEMUXER_H
