//
// Created by rouchie on 2026/3/30.
//

#include "RQFrameMsg.h"

RQFrameMsg::RQFrameMsg(mod_t sender, mod_t recver, cmd_t cmd)
    : RQMsg(sender, recver, cmd) { }