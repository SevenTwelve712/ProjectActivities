cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

# сервер
add_executable(server
    server.cpp
    sort6var.cpp
)

# стандарт C++
target_compile_features(server PRIVATE cxx_std_17)

# если понадобятся pthread
target_link_libraries(server PRIVATE pthread)
