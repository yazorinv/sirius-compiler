cmake_minimum_required(VERSION 3.16)
set(CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-limit-debug-info")
project(sirius_compiler)


include_directories(include)


add_executable(sirius_compiler
        src/main.cpp
        src/compiler.cpp
)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(sirius_compiler PRIVATE nlohmann_json::nlohmann_json)
