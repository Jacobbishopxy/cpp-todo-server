
add_compile_options(-march=native -flto)
add_executable(complex_todo_server Main.cpp Adt.cpp)
target_link_libraries(complex_todo_server ${LIB_UWEBSOCKETS} fmt::fmt)
