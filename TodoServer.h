/**
 * @file:	TodoServer.h
 * @author:	Jacob Xie
 * @date:	2024/11/15 14:33:00 Friday
 * @brief:
 **/

#ifndef __TODOSERVER__H__
#define __TODOSERVER__H__

#include <shared_mutex>
#include <string>

#include "uWebSockets/App.h"

struct Todo
{
    uint id;
    std::string description;
    bool completed;
};

using Todos = std::shared_ptr<std::unordered_map<uint, Todo>>;
using Apps = std::shared_ptr<std::unordered_map<uint, uWS::App>>;

using TodoMutex = std::shared_mutex;

// used for uWS::WebSocket type
struct WsData
{
    std::string_view user_secure_token;
};

class TodoServer
{
public:
    TodoServer(Todos, TodoMutex&);

    void startServer(uint app_num, int port);

    // HTTP API Endpoints
    void getTodo(uWS::HttpResponse<false>* res, uint todoId);
    void deleteTodo(uWS::HttpResponse<false>* res, uint todoId);
    void modifyTodo(uWS::HttpResponse<false>* res, uint todoId, const std::string& description, bool completed);
    void getAllTodos(uWS::HttpResponse<false>* res);

    // WebSocket Handling
    void handleWebSocketConnection(uWS::WebSocket<false, true, WsData>* ws);
    void handleWebSocketMessage(uWS::WebSocket<false, true, WsData>* ws, std::string_view message);
    void handleWebSocketClose(uWS::WebSocket<false, true, WsData>* ws);
    void broadcastMessage(const std::string& topic, const std::string& message);

private:
    Apps m_apps;
    Todos m_todos;
    TodoMutex& m_mutex;
};

using TodoServerPtr = std::shared_ptr<TodoServer>;

#endif  //!__TODOSERVER__H__
