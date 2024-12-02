/**
 * @file:	Adt.cpp
 * @author:	Jacob Xie
 * @date:	2024/12/02 13:36:19 Monday
 * @brief:
 **/

#include "Adt.h"

#include "json.hpp"

void to_json(nlohmann::json& j, const Todo& d)
{
    j = nlohmann::json{
        {"id", d.id},
        {"description", d.description},
        {"completed", d.completed},
    };
}

void from_json(const nlohmann::json& j, Todo& d)
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
