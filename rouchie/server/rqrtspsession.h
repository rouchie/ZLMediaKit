#pragma once

#include "Network/Session.h"
#include "Rtsp/RtspSplitter.h"
#include "Rtsp/RtspMediaSource.h"
#include "Rtsp/RtspMediaSourceImp.h"
#include "Common/MediaSource.h"

constexpr char kRouchieServerName[] = "Rouchie Server 1.0";

class RQRtspSession : public toolkit::Session, public mediakit::RtspSplitter {
public:
    using Ptr = std::shared_ptr<RQRtspSession>;

public:
    RQRtspSession(const toolkit::Socket::Ptr &sock);

    ////Session override////
    void onRecv(const toolkit::Buffer::Ptr &buf) override;
    void onError(const toolkit::SockException &err) override;
    void onManager() override; 

protected:
    /////RtspSplitter override/////
    void onWholeRtspPacket(mediakit::Parser &parser) override;
    void onRtpPacket(const char *data,size_t len) override;

private:
    void handleReq_Options(const mediakit::Parser &parser);
    void handleReq_Describe(const mediakit::Parser &parser);
    void handleReq_Setup(const mediakit::Parser &parser);
    void handleReq_Play(const mediakit::Parser &parser);
    void handleReq_Pause(const mediakit::Parser &parser);
    void handleReq_Teardown(const mediakit::Parser &parser);
    void handleReq_SET_PARAMETER(const mediakit::Parser &parser);

private:
    void sendRtpPacket(const mediakit::RtspMediaSource::RingDataType &pkt);
    void updateRtcpContext(const mediakit::RtpPacket::Ptr &rtp);

    void startListenPeerUdpData(int track_idx);
    void onRcvPeerUdpData(int interleaved, const toolkit::Buffer::Ptr &buf, const struct sockaddr_storage &addr);
    virtual void onRtcpPacket(int track_idx, mediakit::SdpTrack::Ptr &track, const char *data, size_t len);

    bool sendRtspResponse(const std::string &res_code, const std::initializer_list<std::string> &header, const std::string &sdp = "", const char *protocol = "RTSP/1.0");
    bool sendRtspResponse(const std::string &res_code, const mediakit::StrCaseMap &header = mediakit::StrCaseMap(), const std::string &sdp = "", const char *protocol = "RTSP/1.0");
    void send_StreamNotFound();
    void send_SessionNotFound();
    void send_NotAcceptable();

    void onAuthUser(const std::string &realm, const std::string &authorization);
    void onAuthDigest(const std::string &realm, const std::string &auth_md5);
    void onAuthFailed(const std::string &realm, const std::string &why, bool close = true);
    void onAuthSuccess();

    int getTrackIndexByControlUrl(const std::string &control_url) const;
    int getTrackIndexByTrackType(const mediakit::TrackType type) const;

private:
    uint64_t _bytes_usage = 0;
    std::string _content_base;
    int _cseq = 0;
    std::string _sessionid;

    mediakit::Rtsp::eRtpType _rtp_type = mediakit::Rtsp::RTP_Invalid;
    mediakit::RtspMediaSourceImp::Ptr _push_src;
    mediakit::RtspMediaSource::RingType::RingReader::Ptr _play_reader;

    mediakit::MediaInfo _media_info;

    std::string _rtsp_realm;
    std::string _auth_nonce;

    std::vector<mediakit::SdpTrack::Ptr> _sdp_track;
    std::weak_ptr<mediakit::RtspMediaSource> _play_src;

    bool _send_sr_rtcp[2] = {true, true};
    toolkit::Ticker _rtcp_send_tickers[2];
    std::vector<mediakit::RtcpContext::Ptr> _rtcp_context;

    toolkit::Socket::Ptr _rtp_socks[2];
    toolkit::Socket::Ptr _rtcp_socks[2];
    std::unordered_set<int> _udp_connected_flags;
};