#pragma once

#include "RQModuleBase.h"
#include "RQHttpHelper.h"
#include "RQCore.h"
#include "RQHttpHelper.h"
#include "Common/Parser.h"
#include "Http/HttpSession.h"
#include <memory>

using namespace toolkit;
using namespace mediakit;

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

template <typename ModuleType, typename... Args>
RQModuleBase::Ptr CreateHttpApiModule(const Parser &parser, const HttpSession::HttpResponseInvoker &invoker, const SocketHelper::Ptr helper, Args&&... args)
{
    RQHttpHelper::Ptr httpHelper = std::make_shared<RQHttpHelper>(parser, invoker, helper);
    
    // 创建模块实例，完美转发参数
    auto ptr = std::make_shared<ModuleType>(httpHelper, std::forward<Args>(args)...);

    httpHelper->SetModule(ptr);
    
    // 注册到核心管理器
    if (RQCore::INST()) {
        RQCore::INST()->AddModule(ptr->ID(), ptr);
    }
    
    return ptr;
}
