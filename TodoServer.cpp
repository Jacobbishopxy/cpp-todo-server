/**
 * @file:	TodoServer.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/15 14:33:09 Friday
 * @brief:
 **/

#include "TodoServer.h"

#include <fmt/core.h>

#include "include/json.hpp"

// JSON encoding and decoding functions for Todo
void to_json(nlohmann::json& j, const Todo& todo)
{
    j = nlohmann::json{
        {"id", todo.id},
        {"description", todo.description},
        {"completed", todo.completed},
    };
}

void from_json(const nlohmann::json& j, Todo& todo)
{
    j.at("id").get_to(todo.id);
    j.at("description").get_to(todo.description);
    j.at("completed").get_to(todo.completed);
}

TodoServer::TodoServer()
{
    // acquire event loop
    loop = uWS::Loop::get();

    // HTTP routes
    get("/todos", [this](auto* res, auto* req)
        { getAllTodos(res); });

    get("/todo/:id", [this](auto* res, auto* req)
        {
            auto todoId = std::stoi(std::string(req->getParameter(0)));
            getTodo(res, todoId);

            //
        });

    post("/todo", [this](auto* res, auto* req)
         {
             auto isAborted = std::make_shared<bool>(false);

             // Parse JSON body for new TODO details
             res->onData([this, res, isAborted](std::string_view data, bool last)
                         {
                             nlohmann::json body = nlohmann::json::parse(data);
                             std::string description = body["description"];
                             bool completed = body["completed"];

                             if (!*isAborted)
                             {
                                 modifyTodo(res, nextTodoId++, description, completed);
                             }
                             //
                         });

             res->onAborted([isAborted]()
                            { *isAborted = true; });
             //
         });

    del("/todo/:id", [this](auto* res, auto* req)
        {
            auto todoId = std::stoi(std::string(req->getParameter(0)));
            deleteTodo(res, todoId);

            //
        });

    put("/todo/:id", [this](auto* res, auto* req)
        {
            int todoId = std::stoi(std::string(req->getParameter(0)));
            auto isAborted = std::make_shared<bool>(false);

            res->onData([this, res, todoId](std::string_view data, bool last)
                        {
            try {
                nlohmann::json body = nlohmann::json::parse(data);

                std::string description = body.value("description", "");
                bool completed = body.value("completed", false);

                modifyTodo(res, todoId, description, completed);
            } catch (const std::exception &e) {
                res->writeStatus("400 Bad Request")->end("Invalid JSON payload");
            } });

            res->onAborted([isAborted]()
                           { *isAborted = true; });
            //
        });

    // WebSocket route
    ws<WsData>("/*", {
                         .open = [this](auto* ws)
                         { handleWebSocketConnection(ws); },
                         .message = [this](auto* ws, std::string_view message, uWS::OpCode)
                         { handleWebSocketMessage(ws, message); },
                         .close = [this](auto* ws, int, std::string_view)
                         { handleWebSocketClose(ws); },
                     });
}

// HTTP API Implementations

void TodoServer::getTodo(uWS::HttpResponse<false>* res, uint todoId)
{
    std::string msg;
    if (todos.find(todoId) != todos.end())
    {
        nlohmann::json todoJson = todos[todoId];
        msg = fmt::format("getTodo: {}", todoJson.dump());
        res->end(msg);
    }
    else
    {
        msg = fmt::format("getTodo failed: {}", todoId);
        res->end(msg);
    }
    this->broadcastMessage("query", msg);
}

void TodoServer::deleteTodo(uWS::HttpResponse<false>* res, uint todoId)
{
    auto it = this->todos.find(todoId);
    std::string msg;
    if (it != this->todos.end())
    {
        nlohmann::json t = todos[todoId];
        msg = fmt::format("deleteTodo: {}", t.dump());
        this->todos.erase(todoId);
        res->end(msg);
    }
    else
    {
        msg = fmt::format("deleteTodo failed: {}", todoId);
        res->end(msg);
    }
    this->broadcastMessage("mutation", msg);
}

void TodoServer::modifyTodo(uWS::HttpResponse<false>* res, uint todoId, const std::string& description, bool completed)
{
    todos[todoId] = Todo{todoId, description, completed};
    nlohmann::json t = todos[todoId];
    auto msg = fmt::format("modifyTodo: {}", t.dump());

    res->end(msg);
    this->broadcastMessage("mutation", msg);
}

void TodoServer::getAllTodos(uWS::HttpResponse<false>* res)
{
    nlohmann::json allTodos = nlohmann::json::array();
    for (const auto& [id, todo] : todos)
    {
        allTodos.push_back({
            {"id", todo.id},
            {"description", todo.description},
            {"completed", todo.completed},
        });
    }
    auto msg = allTodos.dump();
    res->end(msg);
    // broadcast to ws subscribers
    this->broadcastMessage("query", msg);
}

// WebSocket Handling

void TodoServer::handleWebSocketConnection(uWS::WebSocket<false, true, WsData>* ws)
{
    // ws->subscribe("broadcast");
}

void TodoServer::handleWebSocketMessage(uWS::WebSocket<false, true, WsData>* ws, std::string_view message)
{
    // subscription payload:
    // {"action": "subscribe": "topic": "xxx"}

    try
    {
        // Parse the message as JSON
        nlohmann::json request = nlohmann::json::parse(message);

        // Check if the message requests a subscription
        if (request.contains("action") && request["action"] == "subscribe" && request.contains("topic"))
        {
            std::string topic = request["topic"];
            // Subscribe the WebSocket client to the specified topic
            ws->subscribe(topic);
            // Acknowledge the subscription
            ws->send("Subscribed to topic: " + topic, uWS::OpCode::TEXT);
        }
        else if (request.contains("action") && request["action"] == "unsubscribe" && request.contains("topic"))
        {
            std::string topic = request["topic"];
            // Subscribe the WebSocket client to the specified topic
            ws->unsubscribe(topic);
            // Acknowledge the subscription
            ws->send("Unsubscribed to topic: " + topic, uWS::OpCode::TEXT);
        }
        else
        {
            // Handle other messages (optional)
            ws->send("Invalid request or unsupported action", uWS::OpCode::TEXT);
        }
    }
    catch (const std::exception& e)
    {
        // Handle invalid JSON or other errors
        ws->send("Invalid message format", uWS::OpCode::TEXT);
    }
}

void TodoServer::handleWebSocketClose(uWS::WebSocket<false, true, WsData>* ws)
{
    ws->close();
}

// Broadcast to all WebSocket clients
void TodoServer::broadcastMessage(const std::string& topic, const std::string& message)
{
    this->loop->defer([this, topic, message]()
                      { publish(topic, message, uWS::OpCode::TEXT); });
}
