#include "rqrtmpsession.h"

#include <fmt/format.h>

RQRtmpSession::RQRtmpSession(const toolkit::Socket::Ptr &sock)
    : toolkit::Session(sock) {
    sock->setSendTimeOutSecond(15);
}

RQRtmpSession::~RQRtmpSession() = default;

void RQRtmpSession::onRecv(const toolkit::Buffer::Ptr &buf) {
    onParseRtmp(buf->data(), buf->size());
}

void RQRtmpSession::onError(const toolkit::SockException &err) {
    switch (err.getErrCode()) {
        case toolkit::Err_eof:
        case toolkit::Err_reset: {
            InfoL << fmt::format("{}:用户主动关闭", err.what());
        } break;
        default: {
            ErrorL << fmt::format("{}:{}", static_cast<int>(err.getErrCode()), err.what());
        } break;
    }
}

void RQRtmpSession::onManager() { }

void RQRtmpSession::onSendMedia(const mediakit::RtmpPacket::Ptr &pkt) {
    sendRtmp(pkt->type_id, pkt->stream_index, pkt, pkt->time_stamp, pkt->chunk_id);
}

void RQRtmpSession::onSendRawData(toolkit::Buffer::Ptr buffer) {
    send(std::move(buffer));
}

void RQRtmpSession::onRtmpChunk(mediakit::RtmpPacket::Ptr packet) {
    const auto &chunk = *packet;

    switch (chunk.type_id) {
        case MSG_CMD:
        case MSG_CMD3: {
            AMFDecoder dec(chunk.buffer, 0, chunk.type_id == MSG_CMD3 ? 3 : 0);
            onProcessCmd(dec);
        } break;

        case MSG_DATA:
        case MSG_DATA3: {
            AMFDecoder dec(chunk.buffer, 0, chunk.type_id == MSG_DATA3 ? 3 : 0);
            std::string type = dec.load<std::string>();
            if (type == "@setDataFrame") {
                setMetaData(dec);
            } else {
                TraceP(this) << "unknown notify:" << type;
            }
        } break;

        case MSG_AUDIO:
        case MSG_VIDEO: {
            if (!_set_meta_data) {
                _set_meta_data = true;
                _push_src->setMetaData(_push_metadata ? _push_metadata : mediakit::TitleMeta().getMetadata());
            }
            _push_src->onWrite(std::move(packet));
        } break;
        default: {
        } break;
    }
}

bool RQRtmpSession::close(mediakit::MediaSource &sender)
{
    shutdown(toolkit::SockException(toolkit::Err_shutdown, "close media: " + sender.getUrl()));
    return true;
}

int RQRtmpSession::totalReaderCount(mediakit::MediaSource &sender)
{
    return _push_src ? _push_src->totalReaderCount() : sender.readerCount();
}

mediakit::MediaOriginType RQRtmpSession::getOriginType(mediakit::MediaSource &sender) const
{
    return mediakit::MediaOriginType::rtmp_push;
}

std::string RQRtmpSession::getOriginUrl(mediakit::MediaSource &sender) const
{
    return _media_info.full_url;
}

std::shared_ptr<toolkit::SockInfo> RQRtmpSession::getOriginSock(mediakit::MediaSource &sender) const
{
    return const_cast<RQRtmpSession *>(this)->shared_from_this();
}

toolkit::EventPoller::Ptr RQRtmpSession::getOwnerPoller(mediakit::MediaSource &sender)
{
    return getPoller();
}

void RQRtmpSession::onProcessCmd(AMFDecoder &dec) {
    typedef void (RQRtmpSession::*cmd_function)(AMFDecoder &dec);
    static std::unordered_map<std::string, cmd_function> s_cmd_functions;
    static toolkit::onceToken token([]() {
        s_cmd_functions.emplace("connect", &RQRtmpSession::onCmd_connect);
        s_cmd_functions.emplace("createStream", &RQRtmpSession::onCmd_createStream);
        s_cmd_functions.emplace("play", &RQRtmpSession::onCmd_play);
        s_cmd_functions.emplace("publish", &RQRtmpSession::onCmd_publish);
    });

    std::string method = dec.load<std::string>();
    auto it = s_cmd_functions.find(method);
    if (it == s_cmd_functions.end()) {
        return;
    }

    _recv_req_id = dec.load<double>();

    (this->*it->second)(dec);
}

void RQRtmpSession::onCmd_connect(AMFDecoder &dec) {
    sendChunkSize(60000);
    sendAcknowledgementSize(5000000);
    sendPeerBandwidth(5000000);

    auto params = dec.load<AMFValue>();

    auto tc_url = params["tcUrl"].as_string();
    if (tc_url.empty()) {
        // defaultVhost:默认vhost
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

void RQRtmpSession::onCmd_createStream(AMFDecoder &dec) {
    sendReply("_result", nullptr, double(STREAM_MEDIA));
}

void RQRtmpSession::onCmd_play(AMFDecoder &dec)
{
    dec.load<AMFValue>(); /* NULL */
    _media_info.stream = dec.load<std::string>();
    _media_info.parse(_media_info.getUrl());

    auto f = [this](const mediakit::RtmpMediaSource::Ptr &src) {
        if (src) {
            sendUserControl(CONTROL_STREAM_BEGIN, STREAM_MEDIA);
        }

        sendStatus(
            { "level", "status", "code", "NetStream.Play.Reset", "description", "Resetting and playing.", "details", _media_info.stream, "clientid", "0" });

        if (!src) {
            std::string err_msg = fmt::format("no such stream: {}", _media_info.shortUrl());
            shutdown(toolkit::SockException(toolkit::Err_shutdown, err_msg));
            return;
        }

        sendStatus({ "level", "status", "code", "NetStream.Play.Start", "description", "Started playing.", "details", _media_info.stream, "clientid", "0" });

        AMFEncoder invoke;
        invoke << "|RtmpSampleAccess" << true << true;
        sendResponse(MSG_DATA, invoke.data());

        invoke.clear();
        AMFValue obj(AMF_OBJECT);
        obj.set("code", "NetStream.Data.Start");
        invoke << "onStatus" << obj;
        sendResponse(MSG_DATA, invoke.data());

        sendStatus(
            { "level", "status", "code", "NetStream.Play.PublishNotify", "description", "Now published.", "details", _media_info.stream, "clientid", "0" });

        // metadata
        src->getMetaData([&](const AMFValue &metadata) {
            invoke.clear();
            invoke << "onMetaData" << metadata;
            sendResponse(MSG_DATA, invoke.data());
        });

        // config frame
        src->getConfigFrame([&](const mediakit::RtmpPacket::Ptr &pkt) { onSendMedia(pkt); });

        _ring_reader = src->getRing()->attach(getPoller());
        std::weak_ptr<RQRtmpSession> weak_self = std::static_pointer_cast<RQRtmpSession>(shared_from_this());

        _ring_reader->setGetInfoCB([weak_self]() {
            toolkit::Any ret;
            ret.set(std::static_pointer_cast<toolkit::Session>(weak_self.lock()));
            return ret;
        });

        _ring_reader->setReadCB([weak_self](const mediakit::RtmpMediaSource::RingDataType &pkt) {
            const auto strong_self = weak_self.lock();
            if (!strong_self) {
                return;
            }
            size_t i = 0;
            const auto size = pkt->size();
            strong_self->setSendFlushFlag(false);
            pkt->for_each([&](const mediakit::RtmpPacket::Ptr &rtmp) {
                if (++i == size) {
                    strong_self->setSendFlushFlag(true);
                }
                strong_self->onSendMedia(rtmp);
            });
        });

        src->pause(false);
        _play_src = src;
    };

    std::weak_ptr<RQRtmpSession> weak_self = std::static_pointer_cast<RQRtmpSession>(shared_from_this());
    mediakit::MediaSource::findAsync(_media_info, weak_self.lock(), [weak_self, f](const mediakit::MediaSource::Ptr &src) {
        f(std::dynamic_pointer_cast<mediakit::RtmpMediaSource>(src));
    });
}

void RQRtmpSession::onCmd_publish(AMFDecoder &dec)
{
    std::shared_ptr<toolkit::Ticker> ticker(new toolkit::Ticker);
    std::weak_ptr<RQRtmpSession> weak_self = std::static_pointer_cast<RQRtmpSession>(shared_from_this());

    auto token = std::make_shared<toolkit::onceToken>(nullptr, [ticker, weak_self]() {
        const auto strong_self = weak_self.lock();
        if (strong_self) {
            DebugP(strong_self.get()) << "publish 回复时间:" << ticker->elapsedTime() << "ms";
        }
    });

    dec.load<AMFValue>();/* NULL */
    _media_info.stream = dec.load<std::string>();
    _media_info.parse(_media_info.getUrl());

    if(_media_info.app.empty() || _media_info.stream.empty()){
        const auto err = "rtmp推流url非法";
        sendStatus({ "level", "error",
                     "code", "NetStream.Publish.BadAuth",
                     "description", err,
                     "clientid", "0" });
        shutdown(toolkit::SockException(toolkit::Err_shutdown, err));
        return;
    }

    const auto src = mediakit::MediaSource::find(RTMP_SCHEMA, _media_info.vhost, _media_info.app, _media_info.stream);
    if (src) {
        sendStatus({"level", "error",
                    "code", "NetStream.Publish.BadName",
                    "description", "Already publishing.",
                    "clientid", "0" });
        shutdown(toolkit::SockException(toolkit::Err_shutdown, "Already publishing:"));
        return;
    }

    auto option = mediakit::ProtocolOption();
    // option.enable_mp4 = false;
    // option.enable_hls = false;
    // option.enable_hls_fmp4 = false;
    // option.max_track = 16;

    _push_src = std::make_shared<mediakit::RtmpMediaSourceImp>(_media_info);
    _push_src->setProtocolOption(option);
    _push_src->setListener(std::static_pointer_cast<RQRtmpSession>(shared_from_this()));

    _push_src_ownership = _push_src->getOwnership();
    _continue_push_ms = option.continue_push_ms;

    sendStatus({"level", "status",
                "code", "NetStream.Publish.Start",
                "description", "Started publishing stream.",
                "clientid", "0" });

}

void RQRtmpSession::setMetaData(AMFDecoder &dec)
{
    std::string type = dec.load<std::string>();
    if (type != "onMetaData") {
        throw std::runtime_error("can only set metadata");
    }
    _push_metadata = dec.load<AMFValue>();
    _set_meta_data = false;
}

void RQRtmpSession::sendStatus(const std::initializer_list<std::string> &key_value) {
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
