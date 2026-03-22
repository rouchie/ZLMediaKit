#pragma once

#include "RQRtmpProtocol.h"

#include "Network/Session.h"
#include "Common/MediaSource.h"
#include "Rtmp/RtmpMediaSourceImp.h"
#include "Rtmp/RtmpMediaSource.h"

class RQRtmpSession : public toolkit::Session, public RQRtmpProtocol {
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

private:
    void onProcessCmd(AMFDecoder &dec);
    void onCmd_connect(AMFDecoder &dec);
    void onCmd_createStream(AMFDecoder &dec);
    void onCmd_play(AMFDecoder &dec);

    void sendStatus(const std::initializer_list<std::string> &key_value);
private:
    mediakit::MediaInfo _media_info;
    double _recv_req_id = 0;

    std::weak_ptr<mediakit::RtmpMediaSource> _play_src;
    mediakit::RtmpMediaSource::RingType::RingReader::Ptr _ring_reader;
};