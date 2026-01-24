#include "RQModuleBase.h"

RQModuleBase::RQModuleBase(mod_t id, const EventPoller::Ptr &poller)
    : _modID(id)
{
    _poller = poller ? poller : EventPollerPool::Instance().getPoller();
}

RQModuleBase::~RQModuleBase()
{
}

mod_t RQModuleBase::ID() const
{
    return _modID;
}

const EventPoller::Ptr& RQModuleBase::GetPoller() const
{
    return _poller;
}

void RQModuleBase::UpdateSelf(Ptr self)
{
    this->_self = self;
}

void RQModuleBase::DeleteSelf()
{
    this->_self.reset();
}

void RQModuleBase::Bind(cmd_t id, HandleFunc func)
{
    _mapMsgId2Func[id] = func;
}

int RQModuleBase::OnStart()
{
    return CODE_SUCCESS;
}

int RQModuleBase::OnModuleCallback(const RQMsg::Ptr& msg)
{
    if (!msg) {
        return OnEmptyMsg();
    }

    auto it = _mapMsgId2Func.find(msg->_cmd);
    if (it != _mapMsgId2Func.end()) {
        return it->second(msg);
    }

    return OnUnBindMsg(msg);
}

int RQModuleBase::OnUnBindMsg(const RQMsg::Ptr& msg)
{
    return 0;
}

int RQModuleBase::OnEmptyMsg()
{
    return 0;
}