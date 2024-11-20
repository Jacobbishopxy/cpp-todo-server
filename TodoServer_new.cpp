#include "TodoServer_new.h"
#include <fmt/core.h>
#include "include/json.hpp"

TodoServer::TodoServer()
{
    // 初始化构造函数
    this->nextTodoId = 1;
    this->loop = nullptr;
    this->app = nullptr;
}

void TodoServer::startServer(int port)
{
    // 启动 WebSocket 服务器
    std::cout << "Starting Todo server on port " << port << "..." << std::endl;
    
    app = std::make_shared<uWS::App>();

    // 初始化监听 socket，监听端口
    app->listen(port, [this, port](auto *socket) {
        if (socket) {
            std::cout << "Server successfully listening on port " << port << "!" << std::endl;
        } else {
            std::cerr << "Failed to start server on port " << port << std::endl;
            std::exit(EXIT_FAILURE);  // 监听失败则退出
        }
    });

    app->ws<WsData>("/*", {
        .open = [this](auto *ws) {
            this->handleWebSocketConnection(ws);
        },
        .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
            this->handleWebSocketMessage(ws, message);
        },
        .close = [this](auto *ws, int code, std::string_view message) {
            this->handleWebSocketClose(ws);
        }
    });

    app->run();  // 启动服务器

    std::cout << "Exiting run() on thread: " << std::this_thread::get_id() << "\n";
}

void TodoServer::broadcastMessage(const std::string& topic, const std::string& message)
{
    app->publish(topic, message, uWS::OpCode::TEXT);
}


void TodoServer::handleWebSocketClose(uWS::WebSocket<false, true, WsData>* ws)
{
    std::cout << "Connection closed on thread: " << std::this_thread::get_id() << std::endl;

    // 这里可以添加任何清理操作，例如移除已关闭连接的用户
    // 如果需要，可以访问 ws->getUserData() 获取用户数据
    // ws->getUserData()->user_secure_token;
}

void TodoServer::handleWebSocketConnection(uWS::WebSocket<false, true, WsData>* ws)
{
    std::cout << "New WebSocket connection established on thread: " << std::this_thread::get_id() << std::endl;
    
    // 在连接建立时执行的操作，例如：发送欢迎消息或认证
    ws->send("Welcome to TodoServer!", uWS::OpCode::TEXT);
}

void TodoServer::handleWebSocketMessage(uWS::WebSocket<false, true, WsData>* ws, std::string_view message)
{
    std::cout << "Message received: " << message << " on thread: " << std::this_thread::get_id() << std::endl;
    
    // 在接收到消息时处理，可能是解析消息并做出响应
    ws->send(message, uWS::OpCode::TEXT);  // Echo 回消息
}
