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

int64_t RQModuleBase::Timer(uint64_t interval, const RQMsg::Ptr &msg, bool immediate)
{
    RQTimer::Ptr timer = std::make_shared<RQTimer>(interval, [msg, weak_this = WPtr(shared_from_this())]() {
			if (auto shared_this = weak_this.lock()) {
				shared_this->OnModuleCallback(msg);
				return true;
			}
			return false; // 对象已销毁，停止定时器
        }, immediate, _poller);

    _mapTimers.emplace(timer->ID(), timer);
    return timer->ID();
}

void RQModuleBase::TimerCancel(int64_t timerID)
{
    _mapTimers.erase(timerID);
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