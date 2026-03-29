//
// Created by rouchie on 2026/3/29.
//

#include "RQRtspMediaSourceMuxer.h"

RQRtspMediaSourceMuxer::RQRtspMediaSourceMuxer(const toolkit::EventPoller::Ptr &poller, const mediakit::MediaTuple &tuple, const mediakit::TitleSdp::Ptr &title)
    : mediakit::RtspMuxer(title), _poller(poller) {
    _media_src = std::make_shared<mediakit::RtspMediaSource>(tuple);
}

void RQRtspMediaSourceMuxer::onWrite(std::shared_ptr<mediakit::RtpPacket> in, bool is_key) {
    _media_src->onWrite(in, is_key);
}

bool RQRtspMediaSourceMuxer::addTrack(const mediakit::Track::Ptr &track) {
    return RtspMuxer::addTrack(track);
}

void RQRtspMediaSourceMuxer::addTrackCompleted() {
    RtspMuxer::addTrackCompleted();
    _media_src->setSdp(getSdp());
    _media_src->setListener(this->shared_from_this());

    getRtpRing()->setDelegate(this->shared_from_this());
}

bool RQRtspMediaSourceMuxer::inputFrame(const mediakit::Frame::Ptr &frame) {
    return RtspMuxer::inputFrame(frame);
}

int RQRtspMediaSourceMuxer::totalReaderCount(mediakit::MediaSource &sender) {
    InfoL << sender.getUrl() << ":" << _media_src->readerCount();
    return _media_src->readerCount();
}

void RQRtspMediaSourceMuxer::onReaderChanged(mediakit::MediaSource &sender, int size) {
    InfoL << sender.getUrl() << ":" << size;
    MediaSourceEventInterceptor::onReaderChanged(sender, size);
}

toolkit::EventPoller::Ptr RQRtspMediaSourceMuxer::getOwnerPoller(mediakit::MediaSource &sender) {
    return _poller;
}