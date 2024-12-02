/**
 * @file:	Helpers.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/02 12:10:24 Monday
 * @brief:
 **/

#ifndef __HELPERS__H__
#define __HELPERS__H__

#include "Adt.h"
#include <sstream>

inline uint getMaxId(Todos todos)
{
    if (!todos || todos->empty())
    {
        return 0;  // Return 0 if the map is empty or null
    }

    unsigned int largestKey = 0;  // Start with the smallest integer
    for (const auto& pair : *todos)
    {
        if (pair.first > largestKey)
        {
            largestKey = pair.first;
        }
    }
    return largestKey;
}

inline std::string getTid()
{
    std::stringstream ss;
    auto tid = std::this_thread::get_id();
    ss << tid;
    return ss.str();
}

#endif  //!__HELPERS__H__
