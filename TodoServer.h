/**
 * @file:	TodoServer.h
 * @author:	Jacob Xie
 * @date:	2024/11/15 14:33:00 Friday
 * @brief:
 **/

#ifndef __TODOSERVER__H__
#define __TODOSERVER__H__

#include <string>

#include "uWebSockets/App.h"

struct Todo
{
    uint id;
    std::string description;
    bool completed;
};

struct WsData
{
    std::string_view user_secure_token;
};

class TodoServer : public uWS::App
{
public:
    TodoServer();

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
    std::unordered_map<int, Todo> todos;  // In-memory store for TODOs
    int nextTodoId = 1;                   // ID counter for new TODOs
    std::mutex todosMutex;                // Locks for async modification
    uWS::Loop* loop;                      // Loop for deferring tasks
};

#endif  //!__TODOSERVER__H__
