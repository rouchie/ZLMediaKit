#pragma once

#include "RQModuleBase.h"

class RQCore : public std::enable_shared_from_this<RQCore> {
public:
    using Ptr = std::shared_ptr<RQCore>;
    using WPtr = std::weak_ptr<RQCore>;

public:
    static RQCore::Ptr INST();

public:
    int SendMsg(const RQMsg::Ptr &msg);

    /*
     * 发送延迟消息
     * @param delay 延迟时间，单位毫秒
     * @param msg 消息对象
    */
    int64_t SendDelayMsg(int64_t delay, const RQMsg::Ptr &msg);
    void DelayMsgCancel(int64_t timerID);
    
public:
    void AddModule(mod_t id, RQModuleBase::Ptr module);
    void DelModule(mod_t id);
    void DelModule(RQModuleBase::Ptr module);

private:
    std::recursive_mutex _mutex;
    std::unordered_multimap<mod_t, RQModuleBase::WPtr> _mapModules;

    std::unordered_map<int64_t, RQTimer::Ptr> _mapTimers;
};

inline int SendMsg(const RQMsg::Ptr& msg)
{
    return RQCore::INST()->SendMsg(msg);
}

inline int64_t SendDelayMsg(int64_t delayMS, const RQMsg::Ptr& msg)
{
    return RQCore::INST()->SendDelayMsg(delayMS, msg);
}

inline void DelayMsgCancel(int64_t timerID)
{
    RQCore::INST()->DelayMsgCancel(timerID);
}

