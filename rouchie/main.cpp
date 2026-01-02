/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include <signal.h>
#include <iostream>
#include "Util/File.h"
#include "Util/logger.h"
#include "Util/SSLBox.h"
#include "Util/onceToken.h"
#include "Util/CMD.h"
#include "Network/TcpServer.h"
#include "Network/UdpServer.h"
#include "Poller/EventPoller.h"

#include "Http/HttpRequestSplitter.h"
#include "Rtsp/RtspSplitter.h"
#include "Rtsp/RtspSession.h"
#include "Http/HttpSession.h"

#include "Common/config.h"

#include "spdlog/spdlog.h"

#include "Base/RQCore.h"
#include "MP4ReaderMod.h"
#include "RtspPlayerMod.h"

using namespace toolkit;
using namespace mediakit;

int main(int argc,char *argv[])
{
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

    //auto mp4Mod = CreateModule<MP4ReaderMod>("test.mp4");
    auto rtspMod = CreateModule<RtspPlayerMod>("rtsp://admin:chen5477@192.168.2.111/stream2");

    static semaphore sem;
    signal(SIGINT, [](int) { sem.post(); }); // 设置退出信号
    sem.wait();

    return 0;
}


