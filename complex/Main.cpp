/**
 * @file:	Main.cpp
 * @author:	Jacob Xie
 * @date:	2024/12/02 12:12:15 Monday
 * @brief:
 **/

#include "Builder.hpp"
#include "ISpi.h"

class MySpi : public ISpi
{
public:
    MySpi(Todos todos, TodoMutex& todo_mutex)
        : m_todos(todos), m_mutex(todo_mutex)
    {
    }

    const std::vector<Todo> procQueryTodos() const
    {
        std::shared_lock lock(this->m_mutex);
        std::vector<Todo> all_todo;

        for (const auto& [id, todo] : *this->m_todos.get())
            all_todo.emplace_back(todo);

        return all_todo;
    };

    std::optional<Todo> procQueryTodo(uint todoId) const
    {
        std::shared_lock lock(this->m_mutex);
        if (this->m_todos->find(todoId) != this->m_todos->end())
            return this->m_todos->at(todoId);
        else
            return std::nullopt;
    };

    bool procNewTodo(const Todo& todo)
    {
        std::unique_lock lock(this->m_mutex);
        auto todoId = todo.id;
        if (this->m_todos->find(todoId) != this->m_todos->end())
            return false;
        else
        {
            this->m_todos->insert({todoId, todo});
            return true;
        }
    };

    bool procModifyTodo(const Todo& todo)
    {
        std::unique_lock lock(this->m_mutex);
        auto todoId = todo.id;
        if (this->m_todos->find(todoId) != this->m_todos->end())
        {
            this->m_todos->insert({todoId, todo});
            return true;
        }
        else
            return false;
    };

    bool procDeleteTodo(uint todoId)
    {
        std::unique_lock lock(this->m_mutex);
        if (this->m_todos->find(todoId) != this->m_todos->end())
        {
            this->m_todos->erase(todoId);
            return true;
        }
        else
            return false;
    };

    void procSubscribedMessage(std::string_view message)
    {
        std::cout << "procSubscribedMessage: " << message << std::endl;
    };

private:
    Todos m_todos;
    TodoMutex& m_mutex;
};

int main(int argc, char** argv)
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

    // ================================================================================================

    auto todos = std::make_shared<std::unordered_map<uint, Todo>>();
    auto todo_mutex = std::shared_mutex();
    auto port = 9001;

    // spi
    MySpi my(todos, todo_mutex);

    // lib (singleton
    std::shared_ptr<TodoServer<MySpi>> app = std::make_shared<TodoServer<MySpi>>();
    std::cout << app.get() << std::endl;
    app->registerApp(my);

    for (uint i = 1; i <= workers; ++i)
    {
        std::thread todo_server_t([i, app, port]()
                                  { app->startServer(i, port); });
        todo_server_t.detach();
    }

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return 0;
}
