#pragma once

#include "Base/RQModuleBase.h"
#include "Common/Parser.h"
#include "Http/HttpSession.h"

using namespace toolkit;
using namespace mediakit;

class RQHttpHelper {
public:
    using Ptr = std::shared_ptr<RQHttpHelper>;

public:
    RQHttpHelper(const Parser &parser, const HttpSession::HttpResponseInvoker &invoker, const SocketHelper::Ptr helper)
        : _parser(parser), _invoker(invoker), _helper(helper) {}

    const EventPoller::Ptr GetPoller() const;

    void Response(int code, const HttpSession::KeyValue &header, const std::string &content);

    void SetModule(RQModuleBase::Ptr module);

protected:
    const Parser _parser;
    const HttpSession::HttpResponseInvoker _invoker;
    const SocketHelper::Ptr _helper;

    RQModuleBase::Ptr _module;
};