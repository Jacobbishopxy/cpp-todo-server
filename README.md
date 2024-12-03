# Cpp Todo Server

- [simple](./simple/Main.cpp): a simple application uses `uWS` directly

    ```sh
    ./simple_todo_server --workers 2
    ```

- [complex](./complex/Main.cpp): a complex application uses template builder pattern to include user defined behavior

    ```sh
    ./complex_todo_server --workers 2
    ```

- [library](./library/): header files and libs for user including in other project

    ```sh
    # build package to $HOME/cpp-todo-deploy, see install.sh
    bash install.sh
    ```
