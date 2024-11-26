/**
 * @file:	MapPromiseTest.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/25 15:21:00 Monday
 * @brief:
 **/

#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

int main()
{
    std::unordered_map<std::string, std::unique_ptr<std::promise<int>>> promise_map;
    auto key = "example";

    auto promise_ptr = std::make_unique<std::promise<int>>();
    promise_map[key] = std::move(promise_ptr);

    std::future<int> future = promise_map.at(key)->get_future();

    promise_map.at(key)->set_value(233);

    try
    {
        future.wait();
        std::cout << "Future ready with value: " << future.get() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
    }

    std::cout << "map size: " << promise_map.size() << std::endl;

    promise_map.erase(key);

    std::cout << "map size: " << promise_map.size() << std::endl;

    return 0;
}
