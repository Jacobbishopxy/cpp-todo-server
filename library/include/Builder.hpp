/**
 * @file:	Builder.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/02 08:51:25 Monday
 * @brief:
 **/

#ifndef __GENERATOR__H__
#define __GENERATOR__H__

#include <fmt/format.h>

#include <iostream>
#include <typeinfo>

#include "Adt.h"
#include "Helpers.hpp"
#include "ISpi.h"
#include <nlohmann/json.hpp>

// ================================================================================================
// Builder
// ================================================================================================
#pragma region Builder

template <class SpiType, typename BuilderPatternReturnType>
struct Builder
{
public:
    Builder() = default;
    Builder(Builder&&) noexcept = default;
    Builder& operator=(Builder&&) noexcept = default;

    // disallow copy
    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;

    // register SpiType instance
    BuilderPatternReturnType&& registerApp(SpiType& spi)
    {
        m_spi_ptr = &spi;
        return std::move(static_cast<BuilderPatternReturnType&&>(*this));
    }

    // SPI point get method
    [[nodiscard]] SpiType* getSpiPtr() const noexcept
    {
        return m_spi_ptr;
    }

private:
    SpiType* m_spi_ptr = nullptr;  // SPI interface pointer
};

#pragma endregion Builder

// ================================================================================================
// Server
// ================================================================================================
#pragma region TodoServer

template <IsSpi T>
class TodoServer : public Builder<T, TodoServer<T>>
{
public:
    explicit TodoServer()
        : m_apps(std::make_shared<std::unordered_map<uint, uWS::App>>())
    {
        printInfo();
    }

    void printInfo()
    {
        std::cout << __FUNCTION__ << " running on address: " << this << std::endl;
        std::cout << __FUNCTION__ << " type: " << typeid(this).name() << std::endl;
        std::cout << __FUNCTION__ << " m_apps on address: " << this->m_apps.get() << std::endl;
    }

    void startServer(uint app_num, int port)
    {
        printInfo();
        std::cout << "Starting Todo server on port " << port << "...\n"
                  << std::endl;

        // init uWS app
        this->m_apps->insert({app_num, uWS::App()});

        std::cout << "insert app_num: " << app_num << ", size: " << this->m_apps->size() << std::endl;

        // ================================================================================================
        // get all todos
        // ================================================================================================
        auto get_all = [this](auto* res, auto* req)
        {
            auto all_todos = this->getSpiPtr()->procQueryTodos();
            try
            {
                nlohmann::json j = nlohmann::json(all_todos);
                res->end(j.dump());
            }
            catch (...)
            {
                res->writeStatus("500 Internal Server Error")->end("500 Internal Server Error: An unexpected condition was encountered.");
            }
        };
        this->m_apps->at(app_num).get("/todos", get_all);

        // ================================================================================================
        // get todo
        // ================================================================================================
        auto get_todo = [this](auto* res, auto* req)
        {
            auto todo_id = std::stoi(std::string(req->getParameter(0)));
            auto todo = this->getSpiPtr()->procQueryTodo(todo_id);
            if (todo)
            {
                nlohmann::json j = nlohmann::json(todo.value());
                res->end(j.dump());
            }
            else
            {
                res->writeStatus("400 Bad Request")->end(fmt::format("todo_id: {} not found.", todo_id));
            }
        };
        this->m_apps->at(app_num).get("/todo/:id", get_todo);

        // ================================================================================================
        // create todo
        // ================================================================================================
        auto new_todo = [this](auto* res, auto* req)
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
                        Todo todo = nlohmann::json::parse(buffer);
                        auto success = this->getSpiPtr()->procNewTodo(todo);
                        if (success)
                            res->end("success!");
                        else
                            res->end("failed!");
                    }
                    catch (nlohmann::json::exception& e)
                    {
                        res->writeStatus("400 Bad Request")->end("Invalid JSON payload");
                    }
                    catch (...)
                    {
                        res->writeStatus("500 Internal Server Error")->end("500 Internal Server Error: An unexpected condition was encountered.");
                    }
                }
            };

            res->onData(onData);
            res->onAborted([isAborted]()
                           { *isAborted = true; });
        };
        this->m_apps->at(app_num).post("/todo", new_todo);

        // ================================================================================================
        // modify todo
        // ================================================================================================
        auto modify_todo = [this](auto* res, auto* req)
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
                        Todo todo = nlohmann::json::parse(buffer);
                        auto success = this->getSpiPtr()->procModifyTodo(todo);
                        if (success)
                            res->end("success!");
                        else
                            res->end("failed!");
                    }
                    catch (nlohmann::json::exception& e)
                    {
                        res->writeStatus("400 Bad Request")->end("Invalid JSON payload");
                    }
                    catch (...)
                    {
                        res->writeStatus("500 Internal Server Error")->end("500 Internal Server Error: An unexpected condition was encountered.");
                    }
                }
            };

            res->onData(onData);
            res->onAborted([isAborted]()
                           { *isAborted = true; });
        };
        this->m_apps->at(app_num).put("/todo/:id", modify_todo);

        // ================================================================================================
        // delete todo
        // ================================================================================================
        auto delete_todo = [this](auto* res, auto* req)
        {
            auto todoId = std::stoi(std::string(req->getParameter(0)));
            auto success = this->getSpiPtr()->procDeleteTodo(todoId);
            if (success)
                res->end("success!");
            else
                res->end("failed!");
        };
        this->m_apps->at(app_num).del("/todo/:id", delete_todo);

        // ================================================================================================
        // WebSocket route
        // ================================================================================================
        this->m_apps->at(app_num).template ws<WsData>("/*", {
                                                                .open = [this](auto* ws)
                                                                { this->handleWebSocketConnection(ws); },
                                                                .message = [this](auto* ws, std::string_view message, uWS::OpCode)
                                                                { this->handleWebSocketMessage(ws, message); },
                                                                .close = [this](auto* ws, int, std::string_view)
                                                                { this->handleWebSocketClose(ws); },
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

private:
    void handleWebSocketConnection(uWS::WebSocket<false, true, WsData>* ws)
    {
        auto tid = getTid();
        auto msg = fmt::format("tid: {}", tid);
        ws->send(msg, uWS::OpCode::TEXT);
    }

    void handleWebSocketMessage(uWS::WebSocket<false, true, WsData>* ws, std::string_view message)
    {
        this->getSpiPtr()->procSubscribedMessage(message);
    }

    void handleWebSocketClose(uWS::WebSocket<false, true, WsData>* ws)
    {
        ws->close();
    }

protected:
    Apps m_apps;
};

#pragma endregion TodoServer

#endif  //!__GENERATOR__H__
