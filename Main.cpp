/**
 * @file:	Main.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/15 15:18:55 Friday
 * @brief:
 **/

#include <chrono>
#include <iostream>
#include <random>

#include "TodoServer.h"

void mockServer(TodoServerPtr todoServer);

int main()
{
    try
    {
        TodoServerPtr todo_server = std::make_shared<TodoServer>();

        /**
        // 这么写 http server 的实例化是在主线程上，而 run 方法是在另一个线程上；
        // 结果：client 永远连不上 server，不知道那里阻塞住了
        std::thread todo_server_t([todo_server]()
                                  { todo_server->startServer(9001); });

        todo_server_t.detach();
        */

        std::thread mock_server_t(mockServer, todo_server);
        mock_server_t.detach();

        // 这么写 http server 与 run 方法都在同一线程上，是能跑的
        todo_server->startServer(9001);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return EXIT_SUCCESS;
}

void mockServer(TodoServerPtr todoServer)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 5);  // Random interval between 5 and 15 seconds

    std::cout << "Starting mockServer..." << std::endl;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(dis(gen)));  // Sleep for a random time

        // Get current time
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        // Format time as a string
        std::stringstream timeStream;
        timeStream << std::ctime(&currentTime);  // Convert to string and format it

        // Remove newline character added by ctime
        std::string formattedTime = timeStream.str();
        formattedTime.erase(formattedTime.find('\n'));

        // Trigger the broadcast message on TodoServer
        todoServer->broadcastMessage("random", "Random todo update from mock server: " + formattedTime);
    }
}
