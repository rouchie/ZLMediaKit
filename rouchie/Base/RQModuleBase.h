#pragma once

#include "RQDefines.h"
#include "RQTimer.h"
#include "RQMsg.h"

#include "Poller/EventPoller.h"

using namespace toolkit;

class RQModuleBase {
public:
    using WPtr = std::weak_ptr<RQModuleBase>;
    using Ptr = std::shared_ptr<RQModuleBase>;
    using HandleFunc = std::function<int(const RQMsg::Ptr &)>;

public:
    RQModuleBase(mod_t id, const EventPoller::Ptr &poller = nullptr);
    virtual ~RQModuleBase();

    // 获取模块ID
    mod_t ID() const;

    // 获取模块所属的事件轮询器
    const EventPoller::Ptr& GetPoller() const;

    void UpdateSelf(Ptr self);
    void DeleteSelf();

public:
    // 模块注册完成后会回调该接口
    virtual int OnStart();
    virtual int OnModuleCallback(const RQMsg::Ptr &msg);

protected:
    void Bind(cmd_t cmd, HandleFunc func);

protected:
    virtual int OnUnBindMsg(const RQMsg::Ptr &msg);
    virtual int OnEmptyMsg();

private:
    std::unordered_map<cmd_t, HandleFunc> _mapMsgId2Func;

    mod_t _modID = 0;
    EventPoller::Ptr _poller;

    Ptr _self;
};

template <typename Derived>
class RQModuleHelper
    : public std::enable_shared_from_this<Derived>
    , public RQModuleBase
{
protected:
    RQModuleHelper(mod_t id, const EventPoller::Ptr &poller = nullptr)
        : RQModuleBase(id, poller) {}

protected:
    int64_t Timer(uint64_t interval, const RQMsg::Ptr &msg, bool immediate = false);
    void TimerCancel(int64_t timerID) { _mapTimers.erase(timerID); }

    void Bind(cmd_t cmd, int (Derived::*method)(const RQMsg::Ptr &));
    void Bind(cmd_t cmd, HandleFunc func) { RQModuleBase::Bind(cmd, func); }

private:
    std::unordered_map<int64_t, RQTimer::Ptr> _mapTimers;
    RQModuleBase::Ptr _self;
};

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

