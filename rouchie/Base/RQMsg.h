#pragma once

#include "RQDefines.h"
#include "fmt/format.h"

class RQMsg : public std::enable_shared_from_this<RQMsg> {
public:
    using Ptr = std::shared_ptr<RQMsg>;

public:
    RQMsg(mod_t sender, mod_t recver, cmd_t cmd);

public:
    RQMsg::Ptr Set(int value);
    RQMsg::Ptr Set(int64_t value);
    RQMsg::Ptr Set(double value);
    RQMsg::Ptr Set(const std::string& value);

public:
    mod_t       _sender = 0;
    mod_t       _recver = 0;
    cmd_t       _cmd = 0;

    int64_t     _param0 = 0;
    double      _param1 = 0.;
    std::string _param2;

public:
    static RQMsg::Ptr Build(mod_t recver, cmd_t cmd);
    static RQMsg::Ptr Build(mod_t recver, cmd_t cmd, int64_t value);
    static RQMsg::Ptr Build(mod_t recver, cmd_t cmd, double value);
    static RQMsg::Ptr Build(mod_t recver, cmd_t cmd, const std::string& value);

    static RQMsg::Ptr Build(mod_t sender, mod_t recver, cmd_t cmd, int value);
    static RQMsg::Ptr Build(mod_t sender, mod_t recver, cmd_t cmd, int64_t value);
    static RQMsg::Ptr Build(mod_t sender, mod_t recver, cmd_t cmd, double value);
    static RQMsg::Ptr Build(mod_t sender, mod_t recver, cmd_t cmd, const std::string& value);
};

std::ostream& operator<<(std::ostream& os, const RQMsg::Ptr& obj);

