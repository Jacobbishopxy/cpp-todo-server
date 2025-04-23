/**
 * @file:	Adt.h
 * @author:	Jacob Xie
 * @date:	2024/12/02 11:20:18 Monday
 * @brief:
 **/

#ifndef __ADT__H__
#define __ADT__H__

#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "uWebSockets/App.h"
#include <nlohmann/json.hpp>

struct Todo
{
    uint id;
    std::string description;
    bool completed;
};

inline void to_json(nlohmann::json& j, const Todo& d)
{
    j = nlohmann::json{
        {"id", d.id},
        {"description", d.description},
        {"completed", d.completed},
    };
}

inline void from_json(const nlohmann::json& j, Todo& d)
{
    try
    {
        j.at("id").get_to(d.id);
        j.at("description").get_to(d.description);
        j.at("completed").get_to(d.completed);
    }
    catch (nlohmann::json::exception& e)
    {
        std::cerr << "JSON parsing error in Todo: " << e.what() << '\n';
        throw e;
    }
}

using Todos = std::shared_ptr<std::unordered_map<uint, Todo>>;
using Apps = std::shared_ptr<std::unordered_map<uint, uWS::App>>;
using TodoMutex = std::shared_mutex;

struct WsData
{
    std::string_view user_secure_token;
};

#endif  //!__ADT__H__
