#pragma once

#include "Http/HttpRequestSplitter.h"
#include "Rtmp/Rtmp.h"

class RQRtmpProtocol : public mediakit::HttpRequestSplitter {
public:
    using Ptr = std::shared_ptr<RQRtmpProtocol>; 

public:
	RQRtmpProtocol();
	~RQRtmpProtocol() override;

    void onParseRtmp(const char *data, size_t size);

protected:
    virtual void onSendRawData(toolkit::Buffer::Ptr buffer) = 0;
    virtual void onRtmpChunk(mediakit::RtmpPacket::Ptr chunk_data) = 0;

protected:
    //// HttpRequestSplitter override ////
    ssize_t onRecvHeader(const char *data, size_t len) override { return 0; }
    const char *onSearchPacketTail(const char *data, size_t len) override;

protected:
    void sendAcknowledgement(uint32_t size);
    void sendChunkSize(uint32_t size);
    void sendUserControl(uint16_t event_type, uint32_t event_data);
    void sendRequest(int cmd, const std::string &str);
    void sendResponse(int type, const std::string &str);
    void sendRtmp(uint8_t type, uint32_t stream_index, const std::string &buffer, uint32_t stamp, int chunk_id);
    void sendRtmp(uint8_t type, uint32_t stream_index, const toolkit::Buffer::Ptr &buffer, uint32_t stamp, int chunk_id);
    toolkit::BufferRaw::Ptr obtainBuffer(const void *data = nullptr, size_t len = 0);

private:
    void handle_C1_simple(const char *data);
    const char* handle_C0C1(const char *data, size_t len);
    const char* handle_C2(const char *data, size_t len);
    const char *handle_rtmp(const char *data, size_t len);
    void handle_chunk(mediakit::RtmpPacket::Ptr chunk_data);

protected:
    int _now_stream_index = 0;

private:
    std::function<const char * (const char *data, size_t len)> _next_step_func;

    bool _data_started = false;
    size_t _chunk_size_in = DEFAULT_CHUNK_LEN;
    size_t _chunk_size_out = DEFAULT_CHUNK_LEN;

    uint64_t _bytes_recv = 0;
    uint64_t _bytes_recv_last = 0;
    uint32_t _windows_size = 0;

    std::unordered_map<int, std::pair<mediakit::RtmpPacket::Ptr/*now*/, mediakit::RtmpPacket::Ptr/*last*/> > _map_chunk_data;
    toolkit::ResourcePool<toolkit::BufferRaw> _packet_pool;
};