#pragma once

#include "guibase.h"

#include "Base/RQCoreCreateModule.h"
#include "Extension/Factory.h"
#include "Module/MP4ReaderMod.h"
#include "Utils/RQSaveQueue.h"
#include "Utils/RQJson.h"
#include "server/ShellParser.h"

#include "nlohmann/json.hpp"
#include <cstdarg>
#include <fmt/format.h>
#include <string>
#include <utility>

class RTP2AACFrameMod : public RQModuleHelper<RTP2AACFrameMod>, public mediakit::FrameWriterInterface {
public:
    using Ptr = std::shared_ptr<RTP2AACFrameMod>;

public:
    explicit RTP2AACFrameMod(RQSaveQueue<std::string>::Ptr saveQueue);
    int OnStart() override;

public:
    // mediakit::FrameWriterInterface override
    bool inputFrame(const Frame::Ptr &frame) override;

private:
    int OnCmdStart(const RQMsg::Ptr &msg);
    int OnCmdStop(const RQMsg::Ptr &msg);
    int OnCmdTrigger(const RQMsg::Ptr &msg);

private:
    void handleRtspSource();

private:
    RQSaveQueue<std::string>::Ptr _saveQueue;

    std::string _schema;
    std::string _app;
    std::string _stream;

    mediakit::Track::Ptr _audioTrack;
    mediakit::MediaSource::Ptr _source;
    mediakit::RtpCodec::Ptr _rtpDecoder;
    mediakit::RtspMediaSource::RingType::RingReader::Ptr _rtpRingReader;
};

inline RTP2AACFrameMod::RTP2AACFrameMod(RQSaveQueue<std::string>::Ptr saveQueue)
    : RQModuleHelper(ModID())
    , _saveQueue(std::move(saveQueue)) { }

inline int RTP2AACFrameMod::OnStart() {
    Bind(CMD_START, &RTP2AACFrameMod::OnCmdStart);
    Bind(CMD_STOP, &RTP2AACFrameMod::OnCmdStop);
    Bind(CMD_TRIGGER, &RTP2AACFrameMod::OnCmdTrigger);

    return 0;
}

inline bool RTP2AACFrameMod::inputFrame(const Frame::Ptr &frame) {
    InfoL << "prefix:" << frame->prefixSize() << "-" << frame->dts();
    return true;
}

inline int RTP2AACFrameMod::OnCmdStart(const RQMsg::Ptr &msg) {
    InfoL << "开始播放: " << msg->_param2;
    auto mediaSource = mediakit::MediaSource::find(_schema, "", _app, _stream);
    return 0;
}

inline int RTP2AACFrameMod::OnCmdStop(const RQMsg::Ptr &msg) {
    InfoL << "停止播放: ";
    return 0;
}

inline int RTP2AACFrameMod::OnCmdTrigger(const RQMsg::Ptr &msg) {
    using request_handler = void (RTP2AACFrameMod::*)();
    static std::unordered_map<std::string, request_handler> s_cmd_functions;
    static toolkit::onceToken token([]() {
        s_cmd_functions.emplace("rtsp", &RTP2AACFrameMod::handleRtspSource);
    });

    const nlohmann::json js = ParseJson(msg->_param2);

    _schema = Get(js, "schema", "");
    _app = Get(js, "app", "");
    _stream = Get(js, "stream", "");

    _source = mediakit::MediaSource::find(_schema, "", _app, _stream);
    if (_source) {
        _saveQueue->push(fmt::format("查到流: {}_{}_{}", _schema, _app, _stream));
        const auto it = s_cmd_functions.find(_schema);
        if (it != s_cmd_functions.end()) {
            (this->*(it->second))();
        }
    } else {
        _saveQueue->push(fmt::format("查不到流: {}_{}_{}", _schema, _app, _stream));
    }
    return 0;
}

inline void RTP2AACFrameMod::handleRtspSource() {
    const auto rtsp_src = std::dynamic_pointer_cast<RtspMediaSource>(_source);
    if (!rtsp_src) {
        return;
    }

    const auto sdpTracks = mediakit::SdpParser(rtsp_src->getSdp()).getAvailableTrack();
    for (const auto &track : sdpTracks) {
        if (track->_type != mediakit::TrackAudio || mediakit::getCodecId(track->_codec) != mediakit::CodecAAC) {
            continue;
        }

        _audioTrack = mediakit::Factory::getTrackBySdp(track);
        _audioTrack->addDelegate(shared_from_this());

        _rtpDecoder = mediakit::Factory::getRtpDecoderByCodecId(mediakit::getCodecId(track->_codec));
        _rtpDecoder->addDelegate(_audioTrack);
        InfoL << track->_codec << " " << track->_samplerate << " " << track->_channel << " " << track->_pt << " " << track->_ssrc;
    }

    if (_rtpDecoder == nullptr) {
        _saveQueue->push("不是AAC音频流");
        return;
    }

    std::weak_ptr<RTP2AACFrameMod> weak_ptr = shared_from_this();

    _rtpRingReader = rtsp_src->getRing()->attach(GetPoller(), true);

    _rtpRingReader->setReadCB([weak_ptr](const RtspMediaSource::RingDataType &pack) {
        const auto self = weak_ptr.lock();
        if (!self) {
            return;
        }

        pack->for_each([&](const RtpPacket::Ptr &rtp) {
            if (rtp->type != mediakit::TrackAudio) {
                return;
            }
            self->_rtpDecoder->inputRtp(rtp, false);
        });
    });
}

class GuiAAC : public GuiBase {
public:
    GuiAAC();

    void operator()();

private:
    // 辅助方法：添加日志行（自动维护 lineOffsets）
    void addLog(const std::string& log);
    void addLog(const char *fmt, ...);
    void addRtpData(const mediakit::MediaSource::Ptr &source);

private:
    std::string _schema = "rtsp";
    std::string _app = "live";
    std::string _stream = "stream";

    std::string _btnName = u8"查流";

    ImGuiTextBuffer _buf;
    ImVector<int> _lineOffsets;

    mediakit::MediaSource::Ptr _source;
    mediakit::RtspMediaSource::RingType::RingReader::Ptr _play_reader;
    mediakit::RtpCodec::Ptr _rtpDecoder;

    float _fps = 0.;
    float _fpsCount = 0.;
    RQSaveQueue<std::string>::Ptr _saveQueue;
    RQModuleBase::Ptr _rtp2aac;
};

inline GuiAAC::GuiAAC() {
    _saveQueue = std::make_shared<RQSaveQueue<std::string>>();
    _rtp2aac = CreateModule<RTP2AACFrameMod>(_saveQueue);
}

inline void GuiAAC::operator()() {
    if (_fps == 0.) {
        _fps = ImGui::GetIO().Framerate;
    }

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("AAC");

    ImGui::Text("FPS: %.1f", _fps);

    // 初始化 lineOffsets（只在第一次调用时）
    if (_lineOffsets.empty()) {
        _lineOffsets.push_back(0);
    }

    const float width = ImGui::CalcTextSize("stream").x + 5;
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, width); // 第一列宽度固定

    auto f = [](const char *text, std::string &value) {
        ImGui::Text(text);
        ImGui::NextColumn();
        ImGui::InputText(fmt::format("##{}", text).c_str(), &value);
        ImGui::NextColumn(); // 移到下一行的第一列
    };

    f("schema", _schema);
    f("app", _app);
    f("stream", _stream);

    ImGui::Columns(1); // 恢复单列

    if (ImGui::Button(_btnName.c_str())) {
        addLog("查流: schema=%s, app=%s, stream=%s\n", _schema.c_str(), _app.c_str(), _stream.c_str());
        nlohmann::json js;
        js["schema"] = _schema;
        js["app"] = _app;
        js["stream"] = _stream;
        SendMsg(RQMsg::Build(_rtp2aac->ID(), CMD_TRIGGER, js.dump()));
    }

    if (_fpsCount++ > _fps) {
        _fpsCount = 0;
        std::vector<std::string> v;
        if (_saveQueue->popAll(v) > 0) {
            for (const auto& i : v) {
                addLog(i + "\n");
            }
        }
    }

    // 日志显示区域（设置最小高度确保可见）
    ImGui::Text("日志输出:");
    if (ImGui::BeginChild("logger", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        const char *buf = _buf.begin();
        const char *buf_end = _buf.end();

        ImGuiListClipper clipper;
        clipper.Begin(_lineOffsets.Size);
        while (clipper.Step()) {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
                const char *line_start = buf + _lineOffsets[line_no];
                const char *line_end = (line_no + 1 < _lineOffsets.Size) ? (buf + _lineOffsets[line_no + 1] - 1) : buf_end;
                ImGui::TextUnformatted(line_start, line_end);
            }
        }
        clipper.End();

        ImGui::PopStyleVar();
    }

    ImGui::EndChild();
    ImGui::End();
}

inline void GuiAAC::addLog(const std::string& log) {
    addLog(log.c_str());
}

inline void GuiAAC::addLog(const char *fmt, ...) {
    int old_size = _buf.size();
    va_list args;
    va_start(args, fmt);
    _buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = _buf.size(); old_size < new_size; old_size++) {
        if (_buf[old_size] == '\n') {
            _lineOffsets.push_back(old_size + 1);
        }
    }
}

inline void GuiAAC::addRtpData(const mediakit::MediaSource::Ptr &source) {
    _source = source;

    const auto rtsp_src = std::dynamic_pointer_cast<RtspMediaSource>(source);
    if (!rtsp_src) {
        return;
    }

    const auto sdpTracks = mediakit::SdpParser(rtsp_src->getSdp()).getAvailableTrack();
    for (const auto &track : sdpTracks) {
        if (track->_type != mediakit::TrackAudio) {
            continue;
        }
        _rtpDecoder = mediakit::Factory::getRtpDecoderByCodecId(mediakit::getCodecId(track->_codec));
        InfoL << track->_codec << " " << track->_samplerate << " " << track->_channel << " " << track->_pt << " " << track->_ssrc;
    }

    const auto poller = EventPollerPool::Instance().getPoller();
    poller->async([this, poller, rtsp_src]() {
        _play_reader = rtsp_src->getRing()->attach(EventPollerPool::Instance().getPoller(), true);
        _play_reader->setReadCB([&](const RtspMediaSource::RingDataType &pack) {
            pack->for_each([&](const RtpPacket::Ptr &rtp) {
                if (rtp->type != mediakit::TrackAudio) {
                    return;
                }
                _rtpDecoder->inputRtp(rtp, false);
            });
        });
    });
}
