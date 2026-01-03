#include "RQMsg.h"

RQMsg::RQMsg(mod_t sender, mod_t recver, cmd_t cmd)
    : _sender(sender), _recver(recver), _cmd(cmd)
{
}

RQMsg::Ptr RQMsg::Set(int value)
{
    this->_param0 = value;
    return this->shared_from_this();
}

RQMsg::Ptr RQMsg::Set(int64_t value)
{
    this->_param0 = value;
    return this->shared_from_this();
}

RQMsg::Ptr RQMsg::Set(double value)
{
    this->_param1 = value;
    return this->shared_from_this();
}

RQMsg::Ptr RQMsg::Set(const std::string& value)
{
    this->_param2 = value;
    return this->shared_from_this();
}

RQMsg::Ptr RQMsg::Build(mod_t sender, mod_t recver, cmd_t cmd, int value)
{
    return Build(sender, recver, cmd, (int64_t)value);
}

RQMsg::Ptr RQMsg::Build(mod_t sender, mod_t recver, cmd_t cmd, int64_t value)
{
    RQMsg::Ptr msg = std::make_shared<RQMsg>(sender, recver, cmd);
    msg->_param0 = value;
    return std::move(msg);
}

RQMsg::Ptr RQMsg::Build(mod_t sender, mod_t recver, cmd_t cmd, double value)
{
    RQMsg::Ptr msg = std::make_shared<RQMsg>(sender, recver, cmd);
    msg->_param1 = value;
    return std::move(msg);
}

RQMsg::Ptr RQMsg::Build(mod_t sender, mod_t recver, cmd_t cmd, const std::string& value)
{
    RQMsg::Ptr msg = std::make_shared<RQMsg>(sender, recver, cmd);
    msg->_param2 = value;
    return std::move(msg);
}

RQMsg::Ptr RQMsg::Build(mod_t recver, cmd_t cmd)
{
    RQMsg::Ptr msg = std::make_shared<RQMsg>(MOD_NONAME, recver, cmd);
    return std::move(msg);
}

RQMsg::Ptr RQMsg::Build(mod_t recver, cmd_t cmd, int value)
{
    return Build(recver, cmd, (int64_t)value);
}

RQMsg::Ptr RQMsg::Build(mod_t recver, cmd_t cmd, int64_t value)
{
    RQMsg::Ptr msg = std::make_shared<RQMsg>(MOD_NONAME, recver, cmd);
    msg->_param0 = value;
    return std::move(msg);
}

RQMsg::Ptr RQMsg::Build(mod_t recver, cmd_t cmd, double value)
{
    RQMsg::Ptr msg = std::make_shared<RQMsg>(MOD_NONAME, recver, cmd);
    msg->_param1 = value;
    return std::move(msg);
}

RQMsg::Ptr RQMsg::Build(mod_t recver, cmd_t cmd, const std::string& value)
{
    RQMsg::Ptr msg = std::make_shared<RQMsg>(MOD_NONAME, recver, cmd);
    msg->_param2 = value;
    return std::move(msg);
}

std::ostream& operator<<(std::ostream& os, const RQMsg::Ptr& obj)
{
    auto s = fmt::format("[{}:{}:{}] {},{},{}", GetModuleID(obj->_sender), GetModuleID(obj->_recver), GetCommandID(obj->_cmd), obj->_param0, obj->_param1, obj->_param2);
    os << s;
    return os;
}

