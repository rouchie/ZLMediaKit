//
// Created by rouchie on 2026/3/30.
//

#ifndef ZLMEDIAKIT_RQFRAMEMSG_H
#define ZLMEDIAKIT_RQFRAMEMSG_H

#include "Base/RQMsg.h"
#include "Extension/Frame.h"

class RQFrameMsg : public RQMsg {
public:
    using Ptr = std::shared_ptr<RQFrameMsg>;

    RQFrameMsg(mod_t sender, mod_t recver, cmd_t cmd);

public:
    mediakit::CodecId codecId = mediakit::CodecInvalid;
    mediakit::TrackType trackType = mediakit::TrackInvalid;
    uint64_t dts = 0;
    uint64_t pts = 0;
    toolkit::Buffer::Ptr buffer;
};

#endif // ZLMEDIAKIT_RQFRAMEMSG_H
