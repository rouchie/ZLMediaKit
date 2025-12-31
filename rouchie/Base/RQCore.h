#pragma once

#include "RQModuleBase.h"

class RQCore : public std::enable_shared_from_this<RQCore> {
public:
    using Ptr = std::shared_ptr<RQCore>;

public:
    static RQCore::Ptr INST();

public:
    int SendMsg(const RQMsg::Ptr &msg);
    
public:
    void AddModule(mod_t id, RQModuleBase::Ptr module);
    void DelModule(mod_t id);
    void DelModule(RQModuleBase::Ptr module);

private:
    std::mutex _mutex;
    std::unordered_multimap<mod_t, RQModuleBase::WPtr> _mapModules;
};

template <typename ModuleType, typename... Args>
RQModuleBase::Ptr CreateModule(Args&&... args)
{
    // 创建模块实例，完美转发参数
    auto ptr = std::make_shared<ModuleType>(std::forward<Args>(args)...);
    
    // 注册到核心管理器
    if (RQCore::INST()) {
        RQCore::INST()->AddModule(ptr->ID(), ptr);
    }
    
    return ptr;
}

inline int SendMsg(const RQMsg::Ptr& msg)
{
    return RQCore::INST()->SendMsg(msg);
}
