#pragma once

#include "RQRtmpProtocol.h"

#include "Network/Session.h"
#include "Common/MediaSource.h"

class RQRtmpSession : public toolkit::Session, public RQRtmpProtocol {
public:
    using Ptr = std::shared_ptr<RQRtmpSession>; 

public:
	RQRtmpSession(const toolkit::Socket::Ptr& sock);
	~RQRtmpSession();

	void onRecv(const toolkit::Buffer::Ptr &buf) override;
    void onError(const toolkit::SockException &err) override;
    void onManager() override;

protected:
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
};