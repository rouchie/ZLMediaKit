#pragma once

#include "RQModuleBase.h"
#include "RQCore.h"

template <typename Derived>
class RQModuleHelper
    : public std::enable_shared_from_this<Derived>
    , public RQModuleBase
{
protected:
    RQModuleHelper(mod_t id, const EventPoller::Ptr &poller = nullptr)
        : RQModuleBase(id, poller) {}

protected:
    void SendMessage(mod_t recver, const RQMsg::Ptr &msg) const;

    int64_t Timer(uint64_t interval, const RQMsg::Ptr &msg, bool immediate = false);
    void TimerCancel(int64_t timerID) { _mapTimers.erase(timerID); }

    void Bind(cmd_t cmd, int (Derived::*method)(const RQMsg::Ptr &));
    void Bind(cmd_t cmd, HandleFunc func) override { RQModuleBase::Bind(cmd, func); }

private:
    std::unordered_map<int64_t, RQTimer::Ptr> _mapTimers;
    RQModuleBase::Ptr _self;
};

template <typename Derived>
void RQModuleHelper<Derived>::SendMessage(mod_t recver, const RQMsg::Ptr &msg) const {
    msg->_sender = ID();
    msg->_recver = recver;
    SendMsg(msg);
}

template <typename Derived>
int64_t RQModuleHelper<Derived>::Timer(uint64_t interval, const RQMsg::Ptr &msg, bool immediate)
{
    auto weak_this = WPtr(std::enable_shared_from_this<Derived>::shared_from_this());

    RQTimer::Ptr timer = std::make_shared<RQTimer>(interval, [msg, weak_this]() {
		if (auto shared_this = weak_this.lock()) {
			shared_this->OnModuleCallback(msg);
			return true;
		}
		return false; // 对象已销毁，停止定时器
	}, immediate, GetPoller());

    _mapTimers.emplace(timer->ID(), timer);
    return timer->ID();
}

template<typename Derived>
void RQModuleHelper<Derived>::Bind(cmd_t cmd, int(Derived::*method)(const RQMsg::Ptr&)) {
	WPtr weak_this = std::enable_shared_from_this<Derived>::shared_from_this();

	auto f = [weak_this, method](const RQMsg::Ptr &msg) -> int {
		if (auto shared_this = weak_this.lock()) {
			if (auto derived = std::dynamic_pointer_cast<Derived>(shared_this)) {
				return (derived.get()->*method)(msg);
			}
		}
		return CODE_DESTROYED; // 对象已销毁或类型转换失败
	};

    Bind(cmd, f);
}

