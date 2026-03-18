#include "RQRtmpProtocol.h"

#include "Util/logger.h"
#include "Rtmp/utils.h"
#include "fmt/format.h"

#define C1_DIGEST_SIZE 32
#define C1_KEY_SIZE 128
#define C1_SCHEMA_SIZE 764
#define C1_HANDSHARK_SIZE (RANDOM_LEN + 8)
#define C1_FPKEY_SIZE 30
#define S1_FMS_KEY_SIZE 36
#define S2_FMS_KEY_SIZE 68
#define C1_OFFSET_SIZE 4

RQRtmpProtocol::RQRtmpProtocol()
{
    _packet_pool.setSize(64);
    _next_step_func = [this](const char *data, size_t len) -> const char * { return handle_C0C1(data, len); };
}

RQRtmpProtocol::~RQRtmpProtocol()
{
}

void RQRtmpProtocol::onParseRtmp(const char* data, size_t size)
{
    input(data, size);
}

const char* RQRtmpProtocol::onSearchPacketTail(const char* data, size_t len)
{
    auto next_step_func(std::move(_next_step_func));
    auto ret = next_step_func(data, len);
    if (!_next_step_func) {
        next_step_func.swap(_next_step_func);
    }
    return ret;
}

void RQRtmpProtocol::sendAcknowledgement(uint32_t size)
{
    size = htonl(size);
    std::string acknowledgement((char *) &size, 4);
    sendRequest(MSG_ACK, acknowledgement);
}

void RQRtmpProtocol::sendUserControl(uint16_t event_type, uint32_t event_data)
{
    std::string control;
    event_type = htons(event_type);
    control.append((char *) &event_type, 2);

    event_data = htonl(event_data);
    control.append((char *) &event_data, 4);
    sendRequest(MSG_USER_CONTROL, control);
}

void RQRtmpProtocol::sendRequest(int cmd, const std::string& str)
{
    if (cmd <= MSG_SET_PEER_BW) {
        sendRtmp(cmd, STREAM_CONTROL, str, 0, CHUNK_NETWORK);
    } else {
        sendRtmp(cmd, STREAM_CONTROL, str, 0, CHUNK_SYSTEM);
    }
}

void RQRtmpProtocol::sendResponse(int type, const std::string& str)
{
    if(!_data_started && (type == MSG_DATA)){
        _data_started =  true;
    }
    sendRtmp(type, _now_stream_index, str, 0, _data_started ? CHUNK_CLIENT_REQUEST_AFTER : CHUNK_CLIENT_REQUEST_BEFORE);
}

class BufferPartial : public toolkit::Buffer {
public:
    BufferPartial(const toolkit::Buffer::Ptr &buffer, size_t offset, size_t size) {
        _buffer = buffer;
        _data = buffer->data() + offset;
        _size = size;
    }

    char *data() const override {
        return _data;
    }

    size_t size() const override{
        return _size;
    }

private:
    char *_data;
    size_t _size;
    toolkit::Buffer::Ptr _buffer;
};

void RQRtmpProtocol::sendRtmp(uint8_t type, uint32_t stream_index, const std::string &buffer, uint32_t stamp, int chunk_id)
{
    sendRtmp(type, stream_index, std::make_shared<toolkit::BufferString>(buffer), stamp, chunk_id);
}

void RQRtmpProtocol::sendRtmp(uint8_t type, uint32_t stream_index, const toolkit::Buffer::Ptr &buf, uint32_t stamp, int chunk_id)
{
    bool ext_stamp = stamp >= 0xFFFFFF;

    toolkit::BufferRaw::Ptr buffer_header = obtainBuffer();
    buffer_header->setCapacity(sizeof(mediakit::RtmpHeader));
    buffer_header->setSize(sizeof(mediakit::RtmpHeader));

    mediakit::RtmpHeader *header = (mediakit::RtmpHeader *) buffer_header->data();
    header->fmt = 0;
    header->chunk_id = chunk_id;
    header->type_id = type;
    set_be24(header->time_stamp, ext_stamp ? 0xFFFFFF : stamp);
    set_be24(header->body_size, (uint32_t)buf->size());
    set_le32(header->stream_index, stream_index);
    // 发送rtmp头  [AUTO-TRANSLATED:3c038cd5]
    // Send RTMP header
    onSendRawData(std::move(buffer_header));

    toolkit::BufferRaw::Ptr buffer_ext_stamp;
    if (ext_stamp) {
        // 生成扩展时间戳  [AUTO-TRANSLATED:cd22977a]
        buffer_ext_stamp = obtainBuffer();
        buffer_ext_stamp->setCapacity(4);
        buffer_ext_stamp->setSize(4);
        set_be32(buffer_ext_stamp->data(), stamp);
    }

    toolkit::BufferRaw::Ptr buffer_flags = obtainBuffer();
    buffer_flags->setCapacity(1);
    buffer_flags->setSize(1);
    header = (mediakit::RtmpHeader *) buffer_flags->data();
    header->fmt = 3;
    header->chunk_id = chunk_id;

    size_t offset = 0;

    while (offset < buf->size()) {
        if (offset) {
            onSendRawData(buffer_flags);
        }
        if (ext_stamp) {
            // 扩展时间戳  [AUTO-TRANSLATED:f263b9bf]
            onSendRawData(buffer_ext_stamp);
        }
        size_t chunk = min(_chunk_size_out, buf->size() - offset);
        onSendRawData(std::make_shared<BufferPartial>(buf, offset, chunk));
        offset += chunk;
    }
}

toolkit::BufferRaw::Ptr RQRtmpProtocol::obtainBuffer(const void* data, size_t len)
{
    auto buffer = _packet_pool.obtain2();
    if (data && len) {
        buffer->assign((const char *) data, len);
    }
    return buffer;
}

void RQRtmpProtocol::handle_C1_simple(const char* data)
{
    // S0
    char handshake_head = HANDSHAKE_PLAINTEXT;
    onSendRawData(obtainBuffer(&handshake_head, 1));

    // S1
    mediakit::RtmpHandshake s1(0);
    onSendRawData(obtainBuffer((char *) &s1, C1_HANDSHARK_SIZE));

    // S2, S2是C1的复制
    onSendRawData(obtainBuffer(data + 1, C1_HANDSHARK_SIZE));

    _next_step_func = [this](const char* data, size_t len) -> const char * { return handle_C2(data, len); };
}

const char* RQRtmpProtocol::handle_C0C1(const char* data, size_t len)
{
    if (len < 1 + C1_HANDSHARK_SIZE) {
        //need more data!
        return nullptr;
    }

    if (data[0] != HANDSHAKE_PLAINTEXT) {
        throw std::runtime_error("only plaintext[0x03] handshake supported");
    }

    if (memcmp(data + 5, "\x00\x00\x00\x00", 4) == 0) {
        handle_C1_simple(data);
    } else {
        //throw std::runtime_error("only simple handshake supported");
        // 复杂握手，目前采用简单握手方式处理
        handle_C1_simple(data);
    }

    InfoL << "handle C0C1, len=" << len;

    return data + 1 + C1_HANDSHARK_SIZE;
}

const char* RQRtmpProtocol::handle_C2(const char* data, size_t len)
{
    if (len < C1_HANDSHARK_SIZE) {
        return nullptr;
    }

    InfoL << "handle C2, len=" << len;

    _next_step_func = [this](const char *data, size_t len) -> const char * { return handle_rtmp(data, len); };
    
    return handle_rtmp(data + C1_HANDSHARK_SIZE, len - C1_HANDSHARK_SIZE);
}

static constexpr size_t HEADER_LENGTH[] = { 12, 8, 4, 1 };

const char* RQRtmpProtocol::handle_rtmp(const char* data, size_t len)
{
    auto ptr = data;

    while (len) {
        auto header = (mediakit::RtmpHeader *)ptr; 
        auto header_length = HEADER_LENGTH[header->fmt]; // 这里的header_length包含BasicHeader和MessageHeader
        InfoL << fmt::format("fmt[{}] chunk_id[{}] header_length[{}]", (int)header->fmt, (int)header->chunk_id, header_length);

        int chunk_id = header->chunk_id;
        size_t offset = 0;
        switch (header->chunk_id) {
            case 0: {
                // 表示 Basic Header 长度为 2 字节
                if (len < 2) {
                    return ptr;
                }
                offset = 1;
                chunk_id = 64 + (uint8_t)(ptr[1]);
            } break;
            case 1: {
                // 表示 Basic Header 长度为 3 字节
                if (len < 3) {
                    return ptr;
                }
                offset = 2;
                chunk_id = 64 + (uint8_t)(ptr[1]) + ((uint8_t)(ptr[2]) << 8);
            } break;
            case 2: {
                // 表示 Basic Header 长度为 1 字节
                // 控制信息
            } break;
            default: {
                // 表示 Basic Header 长度为 1 字节
            } break;
        }

        if (len < header_length + offset) {
            return ptr;
        }

        header = (mediakit::RtmpHeader *)(ptr + offset);

        auto &pr = _map_chunk_data[chunk_id];
        auto &now_packet = pr.first;
        auto &last_packet = pr.second;

        if (!now_packet) {
            now_packet = mediakit::RtmpPacket::create();
            if (last_packet) {
                // 手动拷贝成员变量，因为 operator= 是私有的
                now_packet->is_abs_stamp = last_packet->is_abs_stamp;
                now_packet->stream_index = last_packet->stream_index;
                now_packet->body_size = last_packet->body_size;
                now_packet->type_id = last_packet->type_id;
                now_packet->time_stamp = last_packet->time_stamp;
            }
            now_packet->is_abs_stamp = false;
        } else {
            InfoL << "now_packet:" << now_packet;
        }

        auto &chunk = *now_packet;
        chunk.chunk_id = chunk_id;

        switch (header_length) {
            // 需要注意下面都没有break，都是为了利用case穿透的特性，来减少代码重复
            case 12: {
                // 只有这种情况下，才是绝对时间戳，其他情况都是相对于当前的时间戳
                chunk.is_abs_stamp = true;
                chunk.stream_index = load_le32(header->stream_index);
            }
            case 8: {
                chunk.type_id = header->type_id;
                chunk.body_size = load_be24(header->body_size);
            }
            case 4: {
                chunk.ts_field = load_be24(header->time_stamp);
            }
        }

        auto time_stamp = chunk.ts_field;
        if (time_stamp == 0XFFFFFF) {
            if (len < header_length + offset + 4) {
                return ptr;
            }
            time_stamp = load_be32(ptr + offset + header_length);
            offset += 4;
        }

        if (chunk.body_size < chunk.buffer.size()) {
            throw std::runtime_error("非法的 Body Size");
        }
    
        auto more = min(_chunk_size_in, (size_t) chunk.body_size - chunk.buffer.size());
        if (len < header_length + offset + more) {
            return ptr;
        }

        if (more) {
            chunk.buffer.append(ptr + header_length + offset, more);
        }
        ptr += header_length + offset + more;
        len -= header_length + offset + more;

        if (chunk.buffer.size() == chunk.body_size) {
            _now_stream_index = chunk.stream_index;

            chunk.time_stamp = time_stamp + (chunk.is_abs_stamp ? 0 : chunk.time_stamp);
            last_packet = now_packet;
            if (chunk.body_size) {
                handle_chunk(std::move(now_packet));
            } else {
                now_packet = nullptr;
            }
        }
    }

    return ptr;
}

void RQRtmpProtocol::handle_chunk(mediakit::RtmpPacket::Ptr packet)
{
    InfoL << fmt::format(
        "chunk_id[{}] type_id[{}] time_stamp[{}] stream_index[{}] body_size[{}]", (int)packet->chunk_id, (int)packet->type_id, packet->time_stamp, packet->stream_index,
        packet->body_size);

    auto &chunk = *packet;

    switch (chunk.type_id) {
        case MSG_SET_CHUNK: {
            if (chunk.buffer.size() < 4) {
                throw std::runtime_error("MSG_SET_CHUNK :Not enough data");
            }
            _chunk_size_in = load_be32(&chunk.buffer[0]);
            InfoL << fmt::format("chunk_size[{}]", _chunk_size_in);
        } break;
        case MSG_ACK: {
        } break;
        case MSG_USER_CONTROL: {
            InfoL << fmt::format("MSG_USER_CONTROL");
        } break;
        case MSG_WIN_SIZE: {
            _windows_size = min(max(load_be32(&chunk.buffer[0]), 32 * 1024U), 1280 * 1024U);
            InfoL << fmt::format("windows_size[{}]", _windows_size);
        } break;
        case MSG_SET_PEER_BW: {
            uint32_t bandwidth = load_be32(&chunk.buffer[0]);
            uint8_t band_limit_type = chunk.buffer[4];
            InfoL << fmt::format("bandwidth[{}] band_limit_type[{}]", bandwidth, band_limit_type);
        } break;
        case MSG_AGGREGATE: {
            InfoL << fmt::format("MSG_AGGREGATE");
        } break;
        default: {
            _bytes_recv += chunk.size();
            if (_windows_size > 0 && _bytes_recv - _bytes_recv_last >= _windows_size) {
                _bytes_recv_last = _bytes_recv;
                sendAcknowledgement(_bytes_recv);
            }
            onRtmpChunk(std::move(packet));
        } break;
    }
}

