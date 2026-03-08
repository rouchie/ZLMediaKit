#include "Network/TcpServer.h"
#include "Util/logger.h"

#include "Rtsp/RtspSession.h"

#include "Base/RQHttpServer.h"
#include "Module/HttpMod.h"
#include "Module/MP4ReaderMod.h"
#include "Util/onceToken.h"
#include "server/RQRtspSession.h"

#include <fmt/format.h>
#include <signal.h>

using namespace toolkit;
using namespace mediakit;

#ifdef _WIN32
#include <Windows.h>
static onceToken once([](){
    // 设置控制台输出代码页为 UTF-8 (65001)
    SetConsoleOutputCP(CP_UTF8);
});
#endif

int main(int argc, char *argv[])
{
    Logger::Instance().add(std::make_shared<ConsoleChannel>("ConsoleChannel"));
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    auto mp4ReadMod = CreateModule<MP4ReaderMod>("test.mp4", "live", "stream");

    auto rtspSrv = std::make_shared<TcpServer>();
    rtspSrv->start<mediakit::RtspSession>(554, "::");

    auto rqRtspServer = std::make_shared<TcpServer>();
    rqRtspServer->start<RQRtspSession>(50554, "::");

    RQHttpServer::Ptr hs = std::make_shared<RQHttpServer>(80);
    hs->POST<ApiHello>("/aa/bb");
    hs->POST<ApiHello>("/aa/cc");
    hs->Start();

    // std::string url = "rtsp://192.168.2.104:55400/myapp/screenlive";
    // auto rtspMod = CreateModule<RtspPlayerMod>(url);

    static semaphore sem;
    signal(SIGINT, [](int) { sem.post(); }); // 设置退出信号
    sem.wait();

    return 0;
}
