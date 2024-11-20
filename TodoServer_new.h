#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include "uWebSockets/App.h"

struct Todo
{
    uint id;
    std::string description;
    bool completed;
};

// used for uWS::WebSocket type
struct WsData
{
    std::string_view user_secure_token;
};

class TodoServer
{
public:
    TodoServer();  // 构造函数声明

    void startServer(int port);  // 启动服务器
    void broadcastMessage(const std::string& topic, const std::string& message);  // 消息广播

    // 其他 API 函数声明
    void getTodo(uWS::HttpResponse<false>* res, uint todoId);
    void deleteTodo(uWS::HttpResponse<false>* res, uint todoId);
    void modifyTodo(uWS::HttpResponse<false>* res, uint todoId, const std::string& description, bool completed);
    void getAllTodos(uWS::HttpResponse<false>* res);

    // WebSocket 连接、消息和关闭处理
    void handleWebSocketConnection(uWS::WebSocket<false, true, WsData>* ws);
    void handleWebSocketMessage(uWS::WebSocket<false, true, WsData>* ws, std::string_view message);
    void handleWebSocketClose(uWS::WebSocket<false, true, WsData>* ws);

private:
    std::unordered_map<int, Todo> todos;  // 存储 Todo 列表
    int nextTodoId = 1;  // 新 Todo 的 ID
    std::mutex todosMutex;  // 保护 todos 的互斥锁
    uWS::Loop* loop;  // uWS 事件循环
    std::shared_ptr<uWS::App> app;  // WebSocket 应用实例
};

using TodoServerPtr = std::shared_ptr<TodoServer>;

