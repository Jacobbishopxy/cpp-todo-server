#include "App.h"
#include <thread>
#include <vector>
#include <iostream>
#include <atomic>

class WebSocketServer {
public:
    explicit WebSocketServer(int port);  // 构造函数声明

    void run();  // run 方法声明

private:
    int port;
};

// main 函数
int main() {
    const int num_threads = 4;
    std::vector<std::thread> threads;

    // 初始化4个线程，每个线程都有一个WebSocketServer实例
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i]() {
            WebSocketServer server(9010);  // 使用 WebSocketServer 类
            std::cout << "Thread " << i << " started (ID: " << std::this_thread::get_id() << ")\n";
            server.run();  // 每个线程运行一个服务器实例，监听同一个端口
            std::cout << "Thread " << i << " exiting (ID: " << std::this_thread::get_id() << ")\n";
        });
    }

    // 等待所有线程完成
    for (auto &t : threads) {
        t.join();
    }

    return 0;
}

// WebSocketServer 类的定义
WebSocketServer::WebSocketServer(int port) : port(port) {}

void WebSocketServer::run() {
    struct PerSocketData {};

    // 创建 WebSocket 应用实例
    auto app = std::make_shared<uWS::App>();

    // 初始化监听 socket，监听端口
    app->listen(port, [this](auto *socket) {
        if (socket) {
            // 如果监听成功
            std::cout << "Thread " << std::this_thread::get_id() << " listening on port " << port << std::endl;
        } else {
            // 如果监听失败
            std::cerr << "Thread " << std::this_thread::get_id() << " failed to listen on port " << port << std::endl;
            std::exit(EXIT_FAILURE);  // 监听失败则退出
        }
    });

    // 处理 WebSocket 连接
    app->ws<PerSocketData>("/*", {
        .open = [](auto *ws) {
            std::cout << "Connection opened on thread: " << std::this_thread::get_id() << std::endl;
        },
        .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
            std::cout << "Message received on thread: " << std::this_thread::get_id() 
                      << " - " << message << std::endl;
            ws->send(message, opCode);  // Echo back the message
        },
        .close = [](auto *ws, int code, std::string_view message) {
            std::cout << "Connection closed on thread: " << std::this_thread::get_id() 
                      << " with code: " << code << " and message: " << message << std::endl;
        }
    }).run();  // 启动 WebSocket 服务器

    std::cout << "Exiting run() on thread: " << std::this_thread::get_id() << "\n";
};
