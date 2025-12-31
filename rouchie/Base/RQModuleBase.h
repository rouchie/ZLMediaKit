#pragma once

#include "RQDefines.h"
#include "RQTimer.h"
#include "RQMsg.h"

#include "Poller/EventPoller.h"

using namespace toolkit;

class RQModuleBase : public std::enable_shared_from_this<RQModuleBase> {
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

    int64_t Timer(uint64_t interval, const RQMsg::Ptr &msg, bool immediate = false);
    void TimerCancel(int64_t timerID);

public:
    // 模块注册完成后会回调该接口
    virtual int OnStart();
    virtual int OnModuleCallback(const RQMsg::Ptr &msg);

protected:
    virtual int OnUnBindMsg(const RQMsg::Ptr &msg);
    virtual int OnEmptyMsg();

protected:
    void Bind(cmd_t cmd, HandleFunc func);

    template<typename Derived>
    void Bind(cmd_t cmd, int (Derived::*method)(const RQMsg::Ptr &));

protected:
    template <typename... Args>
    RQMsg::Ptr MSG(Args &&...args);

private:
    std::unordered_map<cmd_t, HandleFunc> _mapMsgId2Func;
    std::unordered_map<int64_t, RQTimer::Ptr> _mapTimers;

    mod_t _modID = 0;
    EventPoller::Ptr _poller;
};

template<typename Derived>
void RQModuleBase::Bind(cmd_t cmd, int(Derived::*method)(const RQMsg::Ptr&)) {
	WPtr weak_this = shared_from_this();

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

template <typename... Args>
RQMsg::Ptr RQModuleBase::MSG(Args &&...args)
{
    return RQMsg::Build(ID(), std::forward<Args>(args)...);
}
