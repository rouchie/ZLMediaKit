#include "RQHttpHelper.h"

void RQHttpHelper::SetModule(RQModuleBase::Ptr module)
{
    _module = module;
}

const EventPoller::Ptr RQHttpHelper::GetPoller() const
{
    return _helper->getPoller();
}

void RQHttpHelper::Response(int code, const HttpSession::KeyValue &header, const std::string &content)
{
    _invoker(code, header, content);
    _module.reset();
}