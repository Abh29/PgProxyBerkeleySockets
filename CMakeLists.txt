cmake_minimum_required(VERSION 3.24)
project(pgProxySockets)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall -Werror -Wextra)

add_executable(pgProxySockets
        main.cpp
        src/ServerSelect.cpp
        src/Client.cpp
        src/Client.hpp
        src/Connection.cpp
        src/Connection.hpp
        src/ServerSelect.hpp
        src/IServer.hpp)
