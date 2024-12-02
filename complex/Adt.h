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

#include "App.h"
#include "json.hpp"

struct Todo
{
    uint id;
    std::string description;
    bool completed;
};

void to_json(nlohmann::json& j, const Todo& d);
void from_json(const nlohmann::json& j, Todo& d);

using Todos = std::shared_ptr<std::unordered_map<uint, Todo>>;
using Apps = std::shared_ptr<std::unordered_map<uint, uWS::App>>;
using TodoMutex = std::shared_mutex;

struct WsData
{
    std::string_view user_secure_token;
};

#endif  //!__ADT__H__
