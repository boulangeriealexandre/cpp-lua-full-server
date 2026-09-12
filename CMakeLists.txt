cmake_minimum_required(VERSION 3.15)
project(cpp-lua-full-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Lua REQUIRED)
find_package(pybind11 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(MariaDB REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LUA_INCLUDE_DIR}
    ${MARIADB_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

set(SOURCES
    src/main.cpp
    src/server.cpp
    src/lua_binding.cpp
    src/database.cpp
    src/network.cpp
    src/validation.cpp
)

add_executable(cpp-lua-server ${SOURCES})

target_link_libraries(cpp-lua-server
    ${LUA_LIBRARIES}
    ${MARIADB_LIBRARIES}
    ${Boost_LIBRARIES}
    pybind11::pybind11
)

pybind11_add_module(cpp_lua_py src/python_binding.cpp)
target_link_libraries(cpp_lua_py PRIVATE ${LUA_LIBRARIES} pybind11::pybind11)
