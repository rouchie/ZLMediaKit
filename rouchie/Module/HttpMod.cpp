#include "HttpMod.h"
#include "Base/RQCommandID.h"
#include "Base/RQCore.h"
#include "Base/RQModuleID.h"
#include "Base/RQMsg.h"
#include "Util/logger.h"
#include <fmt/format.h>

static std::atomic<int64_t> s_api_hello_count = {0};

ApiHello::ApiHello(const RQHttpHelper::Ptr helper)
    : RQModuleHelper<ApiHello>(ModID(), helper->GetPoller()), _helper(helper)
{
    _id = ++s_api_hello_count;
    DebugL << _id;
}

ApiHello::~ApiHello()
{
    DebugL;
}

int ApiHello::OnStart()
{
    Bind(CMD_HEARTBEAT, [this](const RQMsg::Ptr&) {
        _helper->Response(200, {}, fmt::format("Hello: {}", _id));
        return 0;
    });

    _helper->Response(200, {}, fmt::format("Hello: {}", _id));
    // SendDelayMsg(1000, RQMsg::Build(ID(), CMD_HEARTBEAT));

    return 0;
}