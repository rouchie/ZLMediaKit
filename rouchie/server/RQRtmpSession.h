#pragma once

#include "RQRtmpProtocol.h"

#include "Network/Session.h"
#include "Common/MediaSource.h"
#include "Rtmp/RtmpMediaSourceImp.h"
#include "Rtmp/RtmpMediaSource.h"

class RQRtmpSession : public toolkit::Session, public RQRtmpProtocol, public mediakit::MediaSourceEvent {
public:
    using Ptr = std::shared_ptr<RQRtmpSession>; 

public:
	explicit RQRtmpSession(const toolkit::Socket::Ptr& sock);
	~RQRtmpSession() override;

	void onRecv(const toolkit::Buffer::Ptr &buf) override;
    void onError(const toolkit::SockException &err) override;
    void onManager() override;

protected:
    void onSendMedia(const mediakit::RtmpPacket::Ptr &pkt);
    void onSendRawData(toolkit::Buffer::Ptr buffer) override;
    void onRtmpChunk(mediakit::RtmpPacket::Ptr packet) override;

    template<typename first, typename second>
    inline void sendReply(const char *str, const first &reply, const second &status) {
        AMFEncoder invoke;
        invoke << str << _recv_req_id << reply << status;
        sendResponse(MSG_CMD, invoke.data());
    }

protected:
    ///////MediaSourceEvent override///////
    bool close(mediakit::MediaSource &sender) override;
    int totalReaderCount(mediakit::MediaSource &sender) override;
    mediakit::MediaOriginType getOriginType(mediakit::MediaSource &sender) const override;
    std::string getOriginUrl(mediakit::MediaSource &sender) const override;
    std::shared_ptr<SockInfo> getOriginSock(mediakit::MediaSource &sender) const override;
    toolkit::EventPoller::Ptr getOwnerPoller(mediakit::MediaSource &sender) override;

private:
    void onProcessCmd(AMFDecoder &dec);
    void onCmd_connect(AMFDecoder &dec);
    void onCmd_createStream(AMFDecoder &dec);
    void onCmd_play(AMFDecoder &dec);
    void onCmd_publish(AMFDecoder &dec);

    void setMetaData(AMFDecoder &dec);

    void sendStatus(const std::initializer_list<std::string> &key_value);
private:
    mediakit::MediaInfo _media_info;
    double _recv_req_id = 0;
    uint32_t _continue_push_ms = 0;

    bool _set_meta_data = false;
    AMFValue _push_metadata;

    std::weak_ptr<mediakit::RtmpMediaSource> _play_src;
    mediakit::RtmpMediaSourceImp::Ptr _push_src;
    std::shared_ptr<void> _push_src_ownership;
    mediakit::RtmpMediaSource::RingType::RingReader::Ptr _ring_reader;
};