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

int main(int argc, char* argv[])
{
    int workers = 1;  // Default workers set to 1

    // Check command-line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        // Check for --workers argument
        if (arg == "--workers" && (i + 1) < argc)
        {
            // Convert the next argument to an integer
            workers = std::stoi(argv[i + 1]);
            ++i;  // Skip the next argument since it's already processed
        }
    }

    // Output the number of workers
    std::cout << "Number of workers: " << workers << std::endl;

    try
    {
        auto todos = std::make_shared<std::unordered_map<uint, Todo>>();
        auto todo_mutex = std::shared_mutex();
        auto port = 9001;

        // singleton
        auto todo_server = std::make_shared<TodoServer>(TodoServer(todos, todo_mutex));

        for (uint i = 1; i <= workers; ++i)
        {
            std::thread todo_server_t([i, todo_server, port]()
                                      { todo_server->startServer(i, port); });
            todo_server_t.detach();
        }

        // mock server thread
        std::thread mock_server_t(mockServer, todo_server);
        mock_server_t.detach();
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
