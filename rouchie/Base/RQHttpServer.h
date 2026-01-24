#pragma once

#include "Base/RQCoreCreateModule.h"
#include "Common/Parser.h"
#include "Http/HttpSession.h"
#include "Network/TcpServer.h"

using namespace toolkit;
using namespace mediakit;

class RQHttpServer {
public:
    using Ptr = std::shared_ptr<RQHttpServer>;

public:
    RQHttpServer(uint16_t port, const std::string &host = "::") : _port(port), _host(host) {}

    int Start();

    template<typename Helper>
    void POST(const std::string& url);

private:
    uint16_t _port = 0;
    std::string _host;

    TcpServer::Ptr _tcpServer;
};

void AddApi(const std::string& url, const std::function<void(const Parser &parser, const HttpSession::HttpResponseInvoker &invoker, const SocketHelper::Ptr helper)> func);

template<typename HttpApiModule>
void RQHttpServer::POST(const std::string& url)
{
    auto f = [](const Parser &parser, const HttpSession::HttpResponseInvoker &invoker, const SocketHelper::Ptr helper) {
        CreateHttpApiModule<HttpApiModule>(parser, invoker, helper);
    };

    AddApi(url, f);
}