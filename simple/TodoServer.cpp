/**
 * @file:	TodoServer.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/15 14:33:09 Friday
 * @brief:
 **/

#include "TodoServer.h"

#include <fmt/core.h>

#include "json.hpp"

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

uint getMaxId(Todos todos)
{
    if (!todos || todos->empty())
    {
        return 0;  // Return 0 if the map is empty or null
    }

    uint largestKey = 0;  // Start with the smallest integer
    for (const auto& pair : *todos)
    {
        if (pair.first > largestKey)
        {
            largestKey = pair.first;
        }
    }
    return largestKey;
}

auto getTid()
{
    std::stringstream ss;
    auto tid = std::this_thread::get_id();
    ss << tid;
    return ss.str();
}

// ================================================================================================
// Server
// ================================================================================================
#pragma region TodoServer

TodoServer::TodoServer(Todos todos, TodoMutex& todo_mutex)
    : m_todos(todos), m_mutex(todo_mutex)
{
    this->m_apps = std::make_shared<std::unordered_map<uint, uWS::App>>();
}

void TodoServer::startServer(uint app_num, int port)
{
    std::cout << "Starting Todo server on port " << port << "..." << std::endl;

    (*this->m_apps).insert({app_num, uWS::App()});

    // HTTP routes
    this->m_apps->at(app_num).get("/todos", [this](auto* res, auto* req)
                                  { getAllTodos(res); });

    // ================================================================================================
    // get_todo
    // ================================================================================================
    auto get_todo = [this](auto* res, auto* req)
    {
        auto todoId = std::stoi(std::string(req->getParameter(0)));
        getTodo(res, todoId);
    };
    this->m_apps->at(app_num).get("/todo/:id", get_todo);

    // ================================================================================================
    // create_todo
    // ================================================================================================
    auto create_dodo = [this](auto* res, auto* req)
    {
        auto isAborted = std::make_shared<bool>(false);
        std::string buffer;
        auto onData = [this,
                       res,
                       isAborted,
                       buffer = std::move(buffer)](std::string_view data, bool last) mutable
        {
            buffer.append(data.data(), data.length());
            if (last)
            {
                try
                {
                    // Parse JSON body for new TODO details
                    nlohmann::json body = nlohmann::json::parse(buffer);
                    std::string description = body["description"];
                    bool completed = body["completed"];
                    auto maxId = getMaxId(this->m_todos);
                    auto newId = maxId + 1;

                    if (!*isAborted)
                        modifyTodo(res, newId, description, completed);
                }
                catch (const std::exception& e)
                {
                    res->writeStatus("400 Bad Request")->end("Invalid JSON payload");
                }
            }
        };

        res->onData(onData);
        res->onAborted([isAborted]()
                       { *isAborted = true; });
    };
    this->m_apps->at(app_num).post("/todo", create_dodo);

    // ================================================================================================
    // delete_todo
    // ================================================================================================
    auto delete_todo = [this](auto* res, auto* req)
    {
        auto todoId = std::stoi(std::string(req->getParameter(0)));
        deleteTodo(res, todoId);
    };
    this->m_apps->at(app_num).del("/todo/:id", delete_todo);

    // ================================================================================================
    // modify_todo
    // ================================================================================================
    auto modify_todo = [this](auto* res, auto* req)
    {
        int todoId = std::stoi(std::string(req->getParameter(0)));
        auto isAborted = std::make_shared<bool>(false);
        std::string buffer;

        auto onData = [this,
                       res,
                       isAborted,
                       todoId,
                       buffer = std::move(buffer)](std::string_view data, bool last) mutable
        {
            buffer.append(data.data(), data.length());
            if (last)
            {
                try
                {
                    nlohmann::json body = nlohmann::json::parse(buffer);
                    std::string description = body.value("description", "");
                    bool completed = body.value("completed", false);

                    if (!*isAborted)
                        modifyTodo(res, todoId, description, completed);
                }
                catch (const std::exception& e)
                {
                    res->writeStatus("400 Bad Request")->end("Invalid JSON payload");
                }
            }
        };
        res->onData(onData);
        res->onAborted([isAborted]()
                       { *isAborted = true; });
    };
    this->m_apps->at(app_num).put("/todo/:id", modify_todo);

    // ================================================================================================
    // WebSocket route
    // ================================================================================================
    this->m_apps->at(app_num).ws<WsData>("/*", {
                                                   .open = [this](auto* ws)
                                                   { handleWebSocketConnection(ws); },
                                                   .message = [this](auto* ws, std::string_view message, uWS::OpCode)
                                                   { handleWebSocketMessage(ws, message); },
                                                   .close = [this](auto* ws, int, std::string_view)
                                                   { handleWebSocketClose(ws); },
                                               });

    // ================================================================================================
    // Run
    // ================================================================================================
    auto listen = [port](auto* token)
    {
        if (token)
        {
            std::cout << "Server listening on port " << port << "!" << std::endl;
        }
        else
        {
            std::cerr << "Failed to start server on port " << port << std::endl;
            exit(EXIT_FAILURE);
        }
    };
    // Listen on the specified port
    this->m_apps->at(app_num).listen(port, listen);

    // Start the server
    this->m_apps->at(app_num).run();
}

// HTTP API Implementations

void TodoServer::getTodo(uWS::HttpResponse<false>* res, uint todoId)
{
    std::shared_lock lock(this->m_mutex);
    std::string msg;
    auto tid = getTid();

    if (this->m_todos->find(todoId) != this->m_todos->end())
    {
        nlohmann::json todoJson = this->m_todos->at(todoId);
        msg = fmt::format("[{}] getTodo: {}", tid, todoJson.dump());
        res->end(msg);
    }
    else
    {
        msg = fmt::format("[{}] getTodo failed: {}", tid, todoId);
        res->end(msg);
    }
    this->broadcastMessage("query", msg);
}

void TodoServer::deleteTodo(uWS::HttpResponse<false>* res, uint todoId)
{
    std::unique_lock lock(this->m_mutex);
    auto it = this->m_todos->find(todoId);
    auto tid = getTid();
    std::string msg;

    if (it != this->m_todos->end())
    {
        nlohmann::json t = this->m_todos->at(todoId);
        msg = fmt::format("[{}] deleteTodo: {}", tid, t.dump());
        this->m_todos->erase(todoId);
        res->end(msg);
    }
    else
    {
        msg = fmt::format("[{}] deleteTodo failed: {}", tid, todoId);
        res->end(msg);
    }
    this->broadcastMessage("mutation", msg);
}

void TodoServer::modifyTodo(uWS::HttpResponse<false>* res, uint todoId, const std::string& description, bool completed)
{
    std::unique_lock lock(this->m_mutex);
    (*this->m_todos).insert({todoId, Todo{todoId, description, completed}});
    auto tid = getTid();
    nlohmann::json t = this->m_todos->at(todoId);
    auto msg = fmt::format("[{}] modifyTodo: {}", tid, t.dump());

    res->end(msg);
    this->broadcastMessage("mutation", msg);
}

void TodoServer::getAllTodos(uWS::HttpResponse<false>* res)
{
    std::shared_lock lock(this->m_mutex);
    nlohmann::json allTodos = nlohmann::json::array();
    for (const auto& [id, todo] : *this->m_todos.get())
    {
        allTodos.push_back({
            {"id", todo.id},
            {"description", todo.description},
            {"completed", todo.completed},
        });
    }
    auto tid = getTid();
    auto msg = fmt::format("[{}] allTodos: {}", tid, allTodos.dump());
    res->end(msg);
    // broadcast to ws subscribers
    this->broadcastMessage("query", msg);
}

// WebSocket Handling

void TodoServer::handleWebSocketConnection(uWS::WebSocket<false, true, WsData>* ws)
{
    auto tid = getTid();
    auto msg = fmt::format("tid: {}", tid);
    ws->send(msg, uWS::OpCode::TEXT);
}

void TodoServer::handleWebSocketMessage(uWS::WebSocket<false, true, WsData>* ws, std::string_view message)
{
    // subscription payload:
    // {"action": "subscribe", "topic": "xxx"}
    // {"action": "unsubscribe", "topic": "xxx"}
    // {"action": "subscriptions"}
    // xxx: all/query/mutation/random

    try
    {
        // Parse the message as JSON
        nlohmann::json request = nlohmann::json::parse(message);

        // Check if the message requests a subscription
        if (request.contains("action") && request["action"] == "subscribe" && request.contains("topic"))
        {
            std::string topic = request["topic"];
            if (topic == "all")
            {
                ws->subscribe("query");
                ws->subscribe("mutation");
                ws->subscribe("random");

                ws->send("Subscribed to all topics: query/mutation/random", uWS::OpCode::TEXT);
            }
            else
            {
                // Subscribe the WebSocket client to the specified topic
                ws->subscribe(topic);
                // Acknowledge the subscription
                ws->send("Subscribed to topic: " + topic, uWS::OpCode::TEXT);
            }
        }
        else if (request.contains("action") && request["action"] == "unsubscribe" && request.contains("topic"))
        {
            std::string topic = request["topic"];
            if (topic == "all")
            {
                ws->unsubscribe("query");
                ws->unsubscribe("mutation");
                ws->unsubscribe("random");

                ws->send("Unsubscribed to all topics: query/mutation/random", uWS::OpCode::TEXT);
            }
            else
            {
                // Unsubscribe the WebSocket client to the specified topic
                ws->unsubscribe(topic);
                // Acknowledge the subscription
                ws->send("Unsubscribed to topic: " + topic, uWS::OpCode::TEXT);
            }
        }
        else if (request.contains("action") && request["action"] == "subscriptions")
        {
            nlohmann::json topics = nlohmann::json::array();
            ws->iterateTopics([&topics, ws](std::string_view topic)
                              { topics.push_back(topic); });
            nlohmann::json topicsJson = topics;
            ws->send("Subscribed topics: " + topicsJson.dump(), uWS::OpCode::TEXT);
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
    for (auto& pair : *this->m_apps)
    {
        auto loop = pair.second.getLoop();
        auto defer = [this, message, topic, &pair]()
        {
            pair.second.publish(topic, message, uWS::OpCode::TEXT);
        };
        loop->defer(defer);
    }
}

#pragma endregion TodoServer
