#pragma once

#include "RQDefines.h"
#include "RQTimer.h"
#include "RQMsg.h"

#include "Poller/EventPoller.h"

using namespace toolkit;

class RQModuleBase : public toolkit::noncopyable {
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
    virtual void Bind(cmd_t cmd, HandleFunc func);

protected:
    virtual int OnUnBindMsg(const RQMsg::Ptr &msg);
    virtual int OnEmptyMsg();

private:
    std::unordered_map<cmd_t, HandleFunc> _mapMsgId2Func;

    mod_t _modID = 0;
    EventPoller::Ptr _poller;

    Ptr _self;
};
