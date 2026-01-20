#include "Network/TcpServer.h"
#include "Poller/EventPoller.h"
#include "Util/logger.h"

#include "Http/HttpSession.h"
#include "Rtsp/RtspSession.h"

#include "Common/config.h"

#include "Base/RQCore.h"
#include "Module/RtspPlayerMod.h"

#include <signal.h>

using namespace toolkit;
using namespace mediakit;

int main(int argc, char *argv[]) {
    Logger::Instance().add(std::make_shared<ConsoleChannel>("ConsoleChannel"));
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastHttpRequest, [](BroadcastHttpRequestArgs) {
        InfoL << parser.url() << "\n" << parser.content();
        return;
        consumed = true;

        auto helper = static_cast<SocketHelper &>(sender).shared_from_this();
        helper->getPoller()->async([parser, invoker, helper]() {
            HttpSession::KeyValue headerOut;
            headerOut["Content-Type"] = std::string("application/json");
            invoker(200, headerOut, "{}");
        });
    });

    auto rtspSrv = std::make_shared<TcpServer>();
    rtspSrv->start<mediakit::RtspSession>(554, "::");

    auto httpSrv = std::make_shared<TcpServer>();
    httpSrv->start<mediakit::HttpSession>(80, "::");

    // auto mp4Mod = CreateModule<MP4ReaderMod>("test.mp4");
    // std::string url = "rtsp://192.168.2.104:55400/myapp/screenlive";
    // auto rtspMod = CreateModule<RtspPlayerMod>(url);

    static semaphore sem;
    signal(SIGINT, [](int) { sem.post(); }); // 设置退出信号
    sem.wait();

    return 0;
}
