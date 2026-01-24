#include "Network/TcpServer.h"
#include "Poller/EventPoller.h"
#include "Util/logger.h"

#include "Http/HttpSession.h"
#include "Rtsp/RtspSession.h"

#include "Common/config.h"

#include "Base/RQCore.h"
#include "Base/RQHttpServer.h"
#include "Module/RtspPlayerMod.h"
#include "Module/HttpMod.h"

#include <fmt/format.h>
#include <signal.h>
#include <sstream>

using namespace toolkit;
using namespace mediakit;

int main(int argc, char *argv[])
{
    Logger::Instance().add(std::make_shared<ConsoleChannel>("ConsoleChannel"));
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    auto rtspSrv = std::make_shared<TcpServer>();
    rtspSrv->start<mediakit::RtspSession>(554, "::");

    RQHttpServer::Ptr hs = std::make_shared<RQHttpServer>(80);
    hs->POST<ApiHello>("/aa/bb");
    hs->POST<ApiHello>("/aa/cc");
    hs->Start();

    // auto mp4Mod = CreateModule<MP4ReaderMod>("test.mp4");
    // std::string url = "rtsp://192.168.2.104:55400/myapp/screenlive";
    // auto rtspMod = CreateModule<RtspPlayerMod>(url);

    static semaphore sem;
    signal(SIGINT, [](int) { sem.post(); }); // 设置退出信号
    sem.wait();

    return 0;
}
