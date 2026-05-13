#include <iostream>
#include <string>
#include <chrono>
#include <uv.h>

// 定义客户端连接数据结构
struct ClientData {
    std::string client_id;
    int message_count;
    std::chrono::steady_clock::time_point connect_time;
};

// TCP服务器回调函数
void on_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        std::cerr << "New connection error: " << uv_strerror(status) << std::endl;
        return;
    }

    uv_tcp_t* client = new uv_tcp_t;
    uv_tcp_init(server->loop, client);
    
    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        // 创建并初始化客户端数据
        ClientData* client_data = new ClientData;
        static int client_counter = 0;
        client_data->client_id = "Client_" + std::to_string(++client_counter);
        client_data->message_count = 0;
        client_data->connect_time = std::chrono::steady_clock::now();
        
        // 将自定义数据绑定到 client handle
        client->data = client_data;
        
        std::cout << "New connection accepted: " << client_data->client_id << std::endl;
        
        // 发送欢迎消息
        std::string welcome = "Welcome to libuv test server! Your ID: " + client_data->client_id + "\n";
        uv_write_t* write_req = new uv_write_t;
        uv_buf_t buf = uv_buf_init(const_cast<char*>(welcome.c_str()), welcome.length());
        
        uv_write(write_req, (uv_stream_t*)client, &buf, 1, [](uv_write_t* req, int status) {
            if (status) {
                std::cerr << "Write error: " << uv_strerror(status) << std::endl;
            }
            delete req;
        });
        
        // 开始读取客户端数据
        uv_read_start((uv_stream_t*)client, 
            [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
                buf->base = new char[suggested_size];
                buf->len = suggested_size;
            },
            [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
                // 获取客户端自定义数据
                ClientData* client_data = static_cast<ClientData*>(stream->data);
                
                if (nread > 0) {
                    client_data->message_count++;
                    std::cout << "[" << client_data->client_id << "] Received (msg #" 
                              << client_data->message_count << "): " 
                              << std::string(buf->base, nread) << std::endl;
                    
                    // 回显收到的数据，并添加前缀
                    std::string response = "Echo [" + client_data->client_id + "]: " + std::string(buf->base, nread);
                    uv_write_t* write_req = new uv_write_t;
                    uv_buf_t write_buf = uv_buf_init(const_cast<char*>(response.c_str()), response.length());
                    uv_write(write_req, stream, &write_buf, 1, [](uv_write_t* req, int status) {
                        delete req;
                    });
                } else if (nread < 0) {
                    if (nread != UV_EOF) {
                        std::cerr << "Read error: " << uv_err_name(nread) << std::endl;
                    }
                    
                    auto now = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                        now - client_data->connect_time).count();
                    
                    std::cout << "[" << client_data->client_id << "] disconnected. "
                              << "Total messages: " << client_data->message_count
                              << ", Duration: " << duration << "s" << std::endl;
                    
                    // 清理客户端数据
                    delete client_data;
                    
                    uv_close((uv_handle_t*)stream, [](uv_handle_t* handle) {
                        delete (uv_tcp_t*)handle;
                    });
                }
                
                delete[] buf->base;
            });
    } else {
        uv_close((uv_handle_t*)client, [](uv_handle_t* handle) {
            delete (uv_tcp_t*)handle;
        });
    }
}

// 定义一个包含自定义数据的结构体
struct TimerData {
    std::string name;
    int custom_value;
    int tick_count;
};

// 定时器回调函数 - 使用 data 字段访问自定义数据
void timer_callback(uv_timer_t* handle) {
    // 从 handle->data 获取自定义数据
    TimerData* data = static_cast<TimerData*>(handle->data);
    
    data->tick_count++;
    std::cout << "Timer [" << data->name << "] tick #" << data->tick_count 
              << ", Custom value: " << data->custom_value << std::endl;
    
    if (data->tick_count >= 5) {
        uv_timer_stop(handle);
        uv_close((uv_handle_t*)handle, [](uv_handle_t* h) {
            // 清理自定义数据
            TimerData* data = static_cast<TimerData*>(h->data);
            delete data;
            delete h;
        });
        std::cout << "Timer [" << data->name << "] stopped after 5 ticks" << std::endl;
    }
}

// 空闲回调函数
void idle_callback(uv_idle_t* handle) {
    std::cout << "Idle callback executed" << std::endl;
    uv_idle_stop(handle);
    uv_close((uv_handle_t*)handle, nullptr);
}

int main() {
    std::cout << "libuv Test Program" << std::endl;
    std::cout << "==================" << std::endl;

    // 获取默认事件循环
    uv_loop_t* loop = uv_default_loop();

    // 创建TCP服务器
    uv_tcp_t server;
    uv_tcp_init(loop, &server);
    
    sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 8080, &addr);
    
    uv_tcp_bind(&server, (const sockaddr*)&addr, 0);
    
    int r = uv_listen((uv_stream_t*)&server, 128, on_connection);
    if (r) {
        std::cerr << "Listen error: " << uv_strerror(r) << std::endl;
        return 1;
    }
    
    std::cout << "TCP Server listening on port 8080" << std::endl;

    // 创建定时器并设置自定义数据
    uv_timer_t* timer1 = new uv_timer_t;
    uv_timer_init(loop, timer1);
    
    // 创建并设置自定义数据
    TimerData* timer1_data = new TimerData{"Heartbeat", 42, 0};
    timer1->data = timer1_data;  // 将自定义数据绑定到 handle
    
    uv_timer_start(timer1, timer_callback, 1000, 1000); // 1秒后开始，每1秒触发一次

    // 创建第二个定时器，演示多个定时器实例
    uv_timer_t* timer2 = new uv_timer_t;
    uv_timer_init(loop, timer2);
    
    TimerData* timer2_data = new TimerData{"StatusCheck", 99, 0};
    timer2->data = timer2_data;
    
    uv_timer_start(timer2, [](uv_timer_t* handle) {
        TimerData* data = static_cast<TimerData*>(handle->data);
        data->tick_count++;
        std::cout << "Timer [" << data->name << "] tick #" << data->tick_count 
                  << ", Custom value: " << data->custom_value << std::endl;
        
        if (data->tick_count >= 3) {
            uv_timer_stop(handle);
            uv_close((uv_handle_t*)handle, [](uv_handle_t* h) {
                TimerData* data = static_cast<TimerData*>(h->data);
                delete data;
                delete h;
            });
        }
    }, 500, 500); // 0.5秒后开始，每0.5秒触发一次

    // 创建空闲处理器
    uv_idle_t idle;
    uv_idle_init(loop, &idle);
    uv_idle_start(&idle, idle_callback);

    std::cout << "Starting event loop..." << std::endl;
    std::cout << "Connect to localhost:8080 to test TCP server" << std::endl;
    
    // 运行事件循环
    uv_run(loop, UV_RUN_DEFAULT);

    // 清理资源
    uv_loop_close(loop);
    
    std::cout << "Program finished" << std::endl;
    return 0;
}