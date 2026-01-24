#include "RQHttpServer.h"

int RQHttpServer::Start()
{
    _tcpServer = std::make_shared<TcpServer>();
    _tcpServer->start<mediakit::HttpSession>(_port, _host);

    return 0;
}

static std::map<std::string,
    std::function<void(const Parser &parser,
        const HttpSession::HttpResponseInvoker &invoker,
        const SocketHelper::Ptr helper)>, StrCaseCompare> g_mHttpApi;

static void InitApi()
{
    NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastHttpRequest, [](BroadcastHttpRequestArgs) {
        consumed = true;

        if (false) {
            std::stringstream ss;
            ss << fmt::format("\n{} {} {}\n", parser.method(), parser.protocol(), parser.url());
            auto header = parser.getHeader();
            for (auto &it : header) {
                ss << it.first << ":" << it.second << "\n";
            }
            if (parser["Content-Type"] == "application/json") {
                ss << fmt::format("\r\n{}", parser.content());
            }

            InfoL << ss.str();
        }

        auto helper = static_cast<SocketHelper &>(sender).shared_from_this();
        auto it = g_mHttpApi.find(parser.url());
        if (g_mHttpApi.end() == it) {
            invoker(404, HttpSession::KeyValue(), "Not Found");
            return;
        }

        it->second(parser, invoker, helper);
    });
}

void AddApi(const std::string& url, const std::function<void(const Parser &parser, const HttpSession::HttpResponseInvoker &invoker, const SocketHelper::Ptr helper)> func)
{
    static onceToken once([]() { InitApi(); });
    g_mHttpApi[url] = func;
}

