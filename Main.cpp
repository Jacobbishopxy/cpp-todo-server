/**
 * @file:	Main.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/15 15:18:55 Friday
 * @brief:
 **/

#include <iostream>

#include "TodoServer.h"

int main()
{
    try
    {
        // Create an instance of the TodoServer
        TodoServer server;

        // Configure the server
        int port = 9001;  // Port to listen on
        std::cout << "Starting Todo server on port " << port << "..." << std::endl;

        // Listen on the specified port
        server.listen(port, [port](auto* token)
                      {
            if (token) {
                std::cout << "Server listening on port " << port << "!" << std::endl;
            } else {
                std::cerr << "Failed to start server on port " << port << std::endl;
                exit(EXIT_FAILURE);
            } });

        // Start the server
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
