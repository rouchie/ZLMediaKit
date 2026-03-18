#include "rqrtmpsession.h"

RQRtmpSession::RQRtmpSession(const toolkit::Socket::Ptr& sock)
    : toolkit::Session(sock)
{
    sock->setSendTimeOutSecond(15);
}

RQRtmpSession::~RQRtmpSession()
{
}

void RQRtmpSession::onRecv(const toolkit::Buffer::Ptr& buf)
{
    onParseRtmp(buf->data(), buf->size());
}

void RQRtmpSession::onError(const toolkit::SockException& err)
{

}

void RQRtmpSession::onManager()
{

}

void RQRtmpSession::onSendRawData(toolkit::Buffer::Ptr buffer)
{
    send(std::move(buffer));
}

void RQRtmpSession::onRtmpChunk(mediakit::RtmpPacket::Ptr packet)
{
    auto &chunk = *packet;

    switch (chunk.type_id) {
        case MSG_CMD:
        case MSG_CMD3: {
            AMFDecoder dec(chunk.buffer, 0, chunk.type_id == MSG_CMD3 ? 3 : 0);
            onProcessCmd(dec);
        } break;
    }
}

void RQRtmpSession::onProcessCmd(AMFDecoder& dec)
{
    typedef void (RQRtmpSession::*cmd_function)(AMFDecoder &dec);
    static std::unordered_map<std::string, cmd_function> s_cmd_functions;
    static toolkit::onceToken token([]() {
        s_cmd_functions.emplace("connect", &RQRtmpSession::onCmd_connect);
        s_cmd_functions.emplace("createStream", &RQRtmpSession::onCmd_createStream);
        s_cmd_functions.emplace("play", &RQRtmpSession::onCmd_play);
    });

    std::string method = dec.load<std::string>();
    auto it = s_cmd_functions.find(method);
    if (it == s_cmd_functions.end()) {
        return;
    }

    _recv_req_id = dec.load<double>();

    (this->*it->second)(dec);
}

void RQRtmpSession::onCmd_connect(AMFDecoder& dec)
{
    auto params = dec.load<AMFValue>();

    auto tc_url = params["tcUrl"].as_string();
    if (tc_url.empty()) {
        // defaultVhost:Ä¬ÈÏvhost
        tc_url = std::string(RTMP_SCHEMA) + "://" + DEFAULT_VHOST + "/" + _media_info.app;
    } else {
        auto pos = tc_url.rfind('?');
        if (pos != std::string::npos) {
            tc_url = tc_url.substr(0, pos);
        }
    }

    _media_info.parse(tc_url);
    _media_info.schema = RTMP_SCHEMA;
    _media_info.app = params["app"].as_string();
    _media_info.protocol = overSsl() ? "rtmps" : "rtmp";

    AMFValue version(AMF_OBJECT);
    version.set("fmsVer", "FMS/3,0,1,123");
    version.set("capabilities", 31.0);
    AMFValue status(AMF_OBJECT);
    status.set("level", "status");
    status.set("code", "NetConnection.Connect.Success");
    status.set("description", "Connection succeeded.");
    status.set("objectEncoding", params["objectEncoding"]);
    sendReply("_result", version, status);

    AMFEncoder invoke;
    invoke << "onBWDone" << 0.0 << nullptr;
    sendResponse(MSG_CMD, invoke.data());
}

void RQRtmpSession::onCmd_createStream(AMFDecoder& dec)
{
    sendReply("_result", nullptr, double(STREAM_MEDIA));
}

void RQRtmpSession::onCmd_play(AMFDecoder &dec)
{
    dec.load<AMFValue>(); /* NULL */
    _media_info.stream = dec.load<std::string>();
    _media_info.parse(_media_info.getUrl());

    sendUserControl(CONTROL_STREAM_BEGIN, STREAM_MEDIA);

    std::string level = "status";
    std::string code = "NetStream.Play.Reset";
    std::string description = "Resetting and playing.";

    sendStatus({ "level", level,
                 "code", code,
                 "description", description,
                 "details", _media_info.stream,
                 "clientid", "0" });

    sendStatus({ "level", "status",
                 "code", "NetStream.Play.Start",
                 "description", "Started playing." ,
                 "details", _media_info.stream,
                 "clientid", "0"});

    AMFEncoder invoke;
    invoke << "|RtmpSampleAccess" << true << true;
    sendResponse(MSG_DATA, invoke.data());

    invoke.clear();
    AMFValue obj(AMF_OBJECT);
    obj.set("code", "NetStream.Data.Start");
    invoke << "onStatus" << obj;
    sendResponse(MSG_DATA, invoke.data());

    sendStatus({ "level", "status",
                 "code", "NetStream.Play.PublishNotify",
                 "description", "Now published." ,
                 "details", _media_info.stream,
                 "clientid", "0"});
}

void RQRtmpSession::sendStatus(const std::initializer_list<std::string>& key_value)
{
    AMFValue status(AMF_OBJECT);
    int i = 0;
    std::string key;
    for (auto &val : key_value) {
        if (++i % 2 == 0) {
            status.set(key, val);
        } else {
            key = val;
        }
    }
    sendReply("onStatus", nullptr, status);
}


