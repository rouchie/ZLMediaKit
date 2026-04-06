//
// Created by rouchie on 2026/4/1.
//

#ifndef ZLMEDIAKIT_RQTRACKMSG_H
#define ZLMEDIAKIT_RQTRACKMSG_H

#include "Base/RQMsg.h"
#include "Extension/Track.h"

class RQTrackMsg : public RQMsg {
public:
    using Ptr = std::shared_ptr<RQTrackMsg>;

    explicit RQTrackMsg(cmd_t cmd);

public:
    mediakit::VideoTrack::Ptr videoTrack;
    mediakit::AudioTrack::Ptr audioTrack;
};

#endif //ZLMEDIAKIT_RQTRACKMSG_H
