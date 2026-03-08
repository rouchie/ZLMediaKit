#include "RQRtspSession.h"
#include "Common/MediaSource.h"
#include "Rtsp/Rtsp.h"
#include "Util/logger.h"
#include <cstddef>
#include <fmt/format.h>
#include "Common/macros.h"
#include "Common/config.h"
#include "Util/MD5.h"
#include "Util/base64.h"

#include <unordered_map>
#include <string>

using namespace mediakit;

RQRtspSession::RQRtspSession(const toolkit::Socket::Ptr &sock)
    : toolkit::Session(sock) {}


void RQRtspSession::onRecv(const toolkit::Buffer::Ptr &buf)
{
    _bytes_usage += buf->size();
    input(buf->data(), buf->size());
}

void RQRtspSession::onError(const toolkit::SockException &err)
{

}

void RQRtspSession::onManager()
{
    // InfoL << fmt::format("Total traffic consumed: {}", _bytes_usage);
}

void RQRtspSession::onWholeRtspPacket(mediakit::Parser &parser)
{
    std::string method = parser.method(); //提取出请求命令字
    _cseq = atoi(parser["CSeq"].data());

    if (_content_base.empty()) {
        mediakit::RtspUrl rtsp;
        rtsp.parse(parser.url());
        _content_base = rtsp._url;
        _media_info.parse(parser.fullUrl());
        _media_info.schema = RTSP_SCHEMA;
        _media_info.protocol = overSsl() ? "rtsps" : "rtsp";
    }

    InfoL << fmt::format("method[{}] url[{}] fullurl[{}] content_base[{}]", parser.method(), parser.url(), parser.fullUrl(), _content_base);

    using rtsp_request_handler = void (RQRtspSession::*)(const mediakit::Parser &parser);
    static std::unordered_map<std::string, rtsp_request_handler> s_cmd_functions;
    static toolkit::onceToken token([]() {
        s_cmd_functions.emplace("OPTIONS", &RQRtspSession::handleReq_Options);
        s_cmd_functions.emplace("DESCRIBE", &RQRtspSession::handleReq_Describe);
        s_cmd_functions.emplace("SETUP", &RQRtspSession::handleReq_Setup);
        s_cmd_functions.emplace("PLAY", &RQRtspSession::handleReq_Play);
        s_cmd_functions.emplace("PAUSE", &RQRtspSession::handleReq_Pause);
        s_cmd_functions.emplace("TEARDOWN", &RQRtspSession::handleReq_Teardown);
        s_cmd_functions.emplace("SET_PARAMETER", &RQRtspSession::handleReq_SET_PARAMETER);
        s_cmd_functions.emplace("GET_PARAMETER", &RQRtspSession::handleReq_SET_PARAMETER);
    });

    auto it = s_cmd_functions.find(method);
    if (it == s_cmd_functions.end()) {
        sendRtspResponse("403 Forbidden");
        throw toolkit::SockException(toolkit::Err_shutdown, StrPrinter << "403 Forbidden:" << method);
    }

    (this->*(it->second))(parser);
    parser.clear();
}

void RQRtspSession::onRtpPacket(const char *data,size_t len)
{

}

void RQRtspSession::handleReq_Options(const mediakit::Parser &parser)
{
    sendRtspResponse("200 OK",{"Public" , "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, ANNOUNCE, RECORD, SET_PARAMETER, GET_PARAMETER"});
}

void RQRtspSession::handleReq_Describe(const mediakit::Parser &parser)
{
    //该请求中的认证信息
    auto authorization = parser["Authorization"];
    std::weak_ptr<RQRtspSession> weak_self = std::static_pointer_cast<RQRtspSession>(shared_from_this());

    //rtsp专属鉴权是否开启事件回调
    auto invoker = [weak_self, authorization](const std::string &realm) {
        auto strong_self = weak_self.lock();
        if (!strong_self) {
            //本对象已经销毁
            return;
        }
        //切换到自己的线程然后执行
        strong_self->async([weak_self, realm, authorization]() {
            auto strong_self = weak_self.lock();
            if (!strong_self) {
                //本对象已经销毁
                return;
            }

            //该流需要rtsp专属认证，开启rtsp专属认证后，将不再触发url通用鉴权认证(on_play)
            strong_self->_rtsp_realm = realm;
            strong_self->onAuthUser(realm, authorization);
        });
    };

    invoker("rouchie");
}

void RQRtspSession::handleReq_Setup(const mediakit::Parser &parser)
{
    static auto getRtpTypeStr = [](const int type) {
        switch (type)
        {
        case Rtsp::RTP_TCP:
            return "TCP";
        case Rtsp::RTP_UDP:
            return "UDP";
        case Rtsp::RTP_MULTICAST:
            return "MULTICAST";
        default:
            return "INVALID";
        }
    };

    InfoL << fmt::format("fullUrl[{}] controlUrl[{}]", parser.fullUrl(), _sdp_track[0]->getControlUrl(_content_base));

    int trackIdx = getTrackIndexByControlUrl(parser.fullUrl());

    SdpTrack::Ptr &trackRef = _sdp_track[trackIdx];
    if (trackRef->_inited) {
        //已经初始化过该Track
        throw toolkit::SockException(toolkit::Err_shutdown, "can not setup one track twice");
    }

    if (_rtp_type == mediakit::Rtsp::RTP_Invalid) {
        auto rtpType = Rtsp::RTP_Invalid;
        auto &transport = parser["Transport"];
        if (transport.find("TCP") != std::string::npos) {
            rtpType = Rtsp::RTP_TCP;
        } else if (transport.find("multicast") != std::string::npos) {
            rtpType = Rtsp::RTP_MULTICAST;
        } else {
            rtpType = Rtsp::RTP_UDP;
        }

        //检查RTP传输类型限制
        GET_CONFIG(int, transportType, Rtsp::kRtpTransportType);
        if (transportType != Rtsp::RTP_Invalid && transportType != rtpType) {
            WarnL << "rtsp client setup transport " << getRtpTypeStr(rtpType) << " but config force transport " << getRtpTypeStr(transportType);
            //配置限定RTSP传输方式，但是客户端握手方式不一致，返回461
            sendRtspResponse("461 Unsupported transport");
            return;
        }

        _rtp_type = rtpType;
    }

    trackRef->_inited = true; //现在初始化

    //允许接收rtp、rtcp包
    RtspSplitter::enableRecvRtp(_rtp_type == Rtsp::RTP_TCP);

    switch (_rtp_type) {
    case Rtsp::RTP_TCP: {
        if (_push_src) {
            // rtsp推流时，interleaved由推流者决定
            auto key_values = Parser::parseArgs(parser["Transport"], ";", "=");
            int interleaved_rtp = -1, interleaved_rtcp = -1;
            if (2 == sscanf(key_values["interleaved"].data(), "%d-%d", &interleaved_rtp, &interleaved_rtcp)) {
                trackRef->_interleaved = interleaved_rtp;
            } else {
                throw toolkit::SockException(toolkit::Err_shutdown, "can not find interleaved when setup of rtp over tcp");
            }
        } else {
            // rtsp播放时，由于数据共享分发，所以interleaved必须由服务器决定
            trackRef->_interleaved = 2 * trackRef->_type;
        }

        auto transport = fmt::format("RTP/AVP/TCP;unicast;interleaved={}-{};ssrc={};", trackRef->_interleaved, trackRef->_interleaved+1, printSSRC(trackRef->_ssrc));
        sendRtspResponse("200 OK", {"Transport", transport});
    } break;
    default:
        break;
    }
}

void RQRtspSession::handleReq_Play(const mediakit::Parser &parser)
{
    if (_sdp_track.empty() || parser["Session"] != _sessionid) {
        send_SessionNotFound();
        throw toolkit::SockException(toolkit::Err_shutdown, _sdp_track.empty() ? "can not find any available track when play" : "session not found when play");
    }

    auto play_src = _play_src.lock();
    if(!play_src){
        send_StreamNotFound();
        throw toolkit::SockException(toolkit::Err_shutdown, "rtsp stream released");
    }

    mediakit::StrCaseMap res_header;

    auto &strRange = parser["Range"];
    if (!strRange.empty()) {
        res_header.emplace("Range", strRange);
    }

    std::vector<TrackType> inited_tracks;
    toolkit::_StrPrinter rtp_info;

    for (size_t i = 0; i < _sdp_track.size(); ++i) {
        auto &track = _sdp_track[i];

        if (track->_inited == false) {
            //为支持播放器播放单一track, 不校验没有发setup的track
            continue;
        }
        inited_tracks.emplace_back(track->_type);
        track->_ssrc = play_src->getSsrc(track->_type);
        track->_seq = play_src->getSequence(track->_type);
        track->_time_stamp = play_src->getTimeStamp(track->_type);

        if (i > 0) {
            rtp_info << ",";
        }

        rtp_info << "url=" << track->getControlUrl(_content_base) << ";"
                 << "seq=" << track->_seq << ";"
                 << "rtptime=" << (int64_t)(track->_time_stamp) * (int64_t)(track->_samplerate/ 1000);
    }

    res_header.emplace("RTP-Info", rtp_info);
    sendRtspResponse("200 OK", res_header);

    //在回复rtsp信令后再恢复播放
    play_src->pause(false);

    if (!_play_reader && _rtp_type != Rtsp::RTP_MULTICAST) {
        std::weak_ptr<RQRtspSession> weak_self = std::static_pointer_cast<RQRtspSession>(shared_from_this());
        _play_reader = play_src->getRing()->attach(getPoller(), true);

        _play_reader->setGetInfoCB([weak_self]() {
            toolkit::Any ret;
            ret.set(std::static_pointer_cast<Session>(weak_self.lock()));
            return ret;
        });

        _play_reader->setDetachCB([weak_self]() {
            auto strong_self = weak_self.lock();
            if (!strong_self) {
                return;
            }
            strong_self->shutdown(toolkit::SockException(toolkit::Err_shutdown, "rtsp ring buffer detached"));
        });

        _play_reader->setReadCB([weak_self](const RtspMediaSource::RingDataType &pack) {
            auto strong_self = weak_self.lock();
            if (!strong_self) {
                return;
            }
            strong_self->sendRtpPacket(pack);
        });
    }
}

void RQRtspSession::handleReq_Pause(const mediakit::Parser &parser)
{
    if (parser["Session"] != _sessionid) {
        send_SessionNotFound();
        throw toolkit::SockException(toolkit::Err_shutdown, "session not found when pause");
    }

    sendRtspResponse("200 OK");
    auto play_src = _play_src.lock();
    if (play_src) {
        play_src->pause(true);
    }
}

void RQRtspSession::handleReq_Teardown(const Parser &parser) {
    _push_src = nullptr;
    //此时回复可能触发broken pipe事件，从而直接触发onError回调；所以需要先把_push_src置空，防止触发断流续推功能
    sendRtspResponse("200 OK");
    throw toolkit::SockException(toolkit::Err_shutdown,"recv teardown request");
}

void RQRtspSession::handleReq_SET_PARAMETER(const Parser &parser)
{
    sendRtspResponse("200 OK");
}

void RQRtspSession::onAuthUser(const std::string &realm, const std::string &authorization)
{
    if(authorization.empty()){
        onAuthFailed(realm, "", false);
        return;
    }

    // Digest username="admin", realm="rouchie", nonce="AGeIMVTgbp8JmGDHH6yFYwmUCd7hPzcI", uri="rtsp://localhost:50554/live/stream", response="55633eb5edd170e401f79b197651ddd6"
    // Basic YWRtaW46YWRtaW4=
    InfoL << fmt::format("realm[{}] authorization[{}]", realm, authorization);

    // 按空格分割 authorization 为前后两部分
    size_t space_pos = authorization.find(' ');
    if (space_pos == std::string::npos) {
        onAuthFailed(realm, "Invalid authorization format");
        return;
    }
    
    std::string authType = authorization.substr(0, space_pos); // 空格前的内容：Basic 或 Digest
    std::string authContent = authorization.substr(space_pos + 1);   // 空格后的内容：认证凭据

    if (authType == "Basic") {
    } else if (authType == "Digest") {
        //md5认证
        onAuthDigest(realm, authContent);
    } else {
        onAuthFailed(realm, fmt::format("Unsupported authorization type: {}", authType), false);
    }
}

void RQRtspSession::onAuthDigest(const std::string &realm, const std::string &auth_md5){
    DebugP(this) << auth_md5;

    auto mapTmp = Parser::parseArgs(auth_md5, ",", "=");
    decltype(mapTmp) map;

    for(auto &pr : mapTmp){
        map[toolkit::trim(std::string(pr.first)," \"")] = toolkit::trim(pr.second," \"");
    }

    auto nonce = map["nonce"];
    auto username = map["username"];
    auto uri = map["uri"];
    auto response = map["response"];

    //check realm
    if(realm != map["realm"]){
        onAuthFailed(realm,StrPrinter << "realm not mached:" << realm << " != " << map["realm"]);
        return;
    }

    //check nonce
    if(_auth_nonce != nonce){
        onAuthFailed(realm,StrPrinter << "nonce not mached:" << nonce << " != " << _auth_nonce);
        return;
    }

    //check username and uri
    if(username.empty() || uri.empty() || response.empty()){
        onAuthFailed(realm,StrPrinter << "username/uri/response empty:" << username << "," << uri << "," << response);
        return;
    }

    auto realInvoker = [this,realm,nonce,uri,username,response](bool ignoreAuth, bool encrypted, const std::string &good_pwd){
        if(ignoreAuth){
            //忽略认证
            TraceP(this) << "auth ignored";
            onAuthSuccess();
            return;
        }
        /*
        response计算方法如下：
        RTSP客户端应该使用username + password并计算response如下:
        (1)当password为MD5编码,则
            response = md5( password:nonce:md5(public_method:url)  );
        (2)当password为ANSI字符串,则
            response= md5( md5(username:realm:password):nonce:md5(public_method:url) );
         */
        auto encrypted_pwd = good_pwd;
        if(!encrypted){
            //提供的是明文密码
            encrypted_pwd = toolkit::MD5(username+ ":" + realm + ":" + good_pwd).hexdigest();
        }

        auto good_response = toolkit::MD5(encrypted_pwd + ":" + nonce + ":" + toolkit::MD5(std::string("DESCRIBE") + ":" + uri).hexdigest()).hexdigest();
        if(strcasecmp(good_response.data(),response.data()) == 0){
            //认证成功！md5不区分大小写
            onAuthSuccess();
        }else{
            //认证失败！
            onAuthFailed(realm, StrPrinter << "password mismatch when md5 auth:" << good_response << " != " << response );
        }
    };

    realInvoker(false, false, "admin");
}

void RQRtspSession::onAuthSuccess() {
    std::weak_ptr<RQRtspSession> weak_self = std::static_pointer_cast<RQRtspSession>(shared_from_this());

    MediaSource::findAsync(_media_info, weak_self.lock(), [weak_self](const MediaSource::Ptr &src){
        auto strong_self = weak_self.lock();
        if(!strong_self){
            return;
        }

        auto rtsp_src = std::dynamic_pointer_cast<RtspMediaSource>(src);
        if (!rtsp_src) {
            //未找到相应的MediaSource
            std::string err = fmt::format("no such stream: {}", strong_self->_media_info.shortUrl());
            strong_self->send_StreamNotFound();
            strong_self->shutdown(toolkit::SockException(toolkit::Err_shutdown,err));
            return;
        }

        InfoL << rtsp_src->getSdp();

        strong_self->_sdp_track = mediakit::SdpParser(rtsp_src->getSdp()).getAvailableTrack();
        if (strong_self->_sdp_track.empty()) {
            //未找到可用的Track
            std::string err = fmt::format("no available track: {}", strong_self->_media_info.shortUrl());
            strong_self->send_StreamNotFound();
            strong_self->shutdown(toolkit::SockException(toolkit::Err_shutdown, err));
            return;
        }

        strong_self->_sessionid = toolkit::makeRandStr(12);
        for(auto &track : strong_self->_sdp_track){
            auto ssrc = rtsp_src->getSsrc(track->_type);
            auto seq = rtsp_src->getSequence(track->_type);
            auto time_stamp = rtsp_src->getTimeStamp(track->_type);

            InfoL << fmt::format("ssrc[{}] seq[{}] time_stamp[{}]", ssrc, seq, time_stamp);

            track->_ssrc = ssrc;
            track->_seq = seq;
            track->_time_stamp = time_stamp;
        }

        strong_self->_play_src = rtsp_src;

        strong_self->sendRtspResponse("200 OK",
            {"Content-Base", strong_self->_content_base + "/"},
            rtsp_src->getSdp());
    });
}

void RQRtspSession::onAuthFailed(const std::string &realm, const std::string &why, bool close)
{
    ::toolkit::mINI::Instance()[Rtsp::kAuthBasic] = false; 
    GET_CONFIG(bool, authBasic, Rtsp::kAuthBasic);

    if (!authBasic) {
        // 我们需要客户端优先以md5方式认证
        _auth_nonce = toolkit::makeRandStr(32);
        sendRtspResponse("401 Unauthorized", { "WWW-Authenticate", StrPrinter << "Digest realm=\"" << realm << "\",nonce=\"" << _auth_nonce << "\"" });
    } else {
        // 当然我们也支持base64认证,但是我们不建议这样做
        sendRtspResponse("401 Unauthorized", { "WWW-Authenticate", StrPrinter << "Basic realm=\"" << realm << "\"" });
    }

    if (close) {
        shutdown(toolkit::SockException(toolkit::Err_shutdown, StrPrinter << "401 Unauthorized:" << why));
    }
}

int RQRtspSession::getTrackIndexByControlUrl(const std::string &control_url)
{
    for (size_t i = 0; i < _sdp_track.size(); ++i) {
        if (control_url.find(_sdp_track[i]->getControlUrl(_content_base)) == 0) {
            return i;
        }
    }
    if (_sdp_track.size() == 1) {
        return 0;
    }
    throw toolkit::SockException(toolkit::Err_shutdown, StrPrinter << "no such track with control url:" << control_url);
}

void RQRtspSession::sendRtpPacket(const mediakit::RtspMediaSource::RingDataType &pkt)
{
    switch (_rtp_type) {
        case Rtsp::RTP_TCP: {
            setSendFlushFlag(false);
            pkt->for_each([&](const RtpPacket::Ptr &rtp) { send(rtp); });
            flushAll();
            setSendFlushFlag(true);
        } break;
        default: break;
    }
}

bool RQRtspSession::sendRtspResponse(const std::string &res_code, const std::initializer_list<std::string> &header, const std::string &sdp, const char *protocol)
{
    std::string key;
    mediakit::StrCaseMap header_map;
    int i = 0;
    for(auto &val : header){
        if(++i % 2 == 0){
            header_map.emplace(key,val);
        }else{
            key = val;
        }
    }
    return sendRtspResponse(res_code, header_map, sdp, protocol);
}

static std::string dateStr()
{
    char buf[64];
    time_t tt = time(NULL);
    strftime(buf, sizeof buf, "%a, %b %d %Y %H:%M:%S GMT", gmtime(&tt));
    return buf;
}

bool RQRtspSession::sendRtspResponse(const std::string &res_code, const mediakit::StrCaseMap &header_const, const std::string &sdp, const char *protocol)
{
    auto header = header_const;
    header.emplace("CSeq",StrPrinter << _cseq);

    if(!_sessionid.empty()){
        header.emplace("Session", _sessionid);
    }

    header.emplace("Server", kRouchieServerName);
    header.emplace("Date", dateStr());

    if(!sdp.empty()){
        header.emplace("Content-Length",StrPrinter << sdp.size());
        header.emplace("Content-Type","application/sdp");
    }

    toolkit::_StrPrinter printer;
    printer << protocol << " " << res_code << "\r\n";
    for (auto &pr : header){
        printer << pr.first << ": " << pr.second << "\r\n";
    }

    printer << "\r\n";

    if(!sdp.empty()){
        printer << sdp;
    }
//	DebugP(this) << printer;
    return send(std::make_shared<toolkit::BufferString>(std::move(printer))) > 0 ;
}

void RQRtspSession::send_StreamNotFound()
{
    sendRtspResponse("404 Stream Not Found",{"Connection","Close"});
}

void RQRtspSession::send_SessionNotFound()
{
    sendRtspResponse("454 Session Not Found",{"Connection","Close"});
}

