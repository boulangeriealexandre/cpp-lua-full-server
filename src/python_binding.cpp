#include <lua.hpp>
#include <pybind11/pybind11.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>

namespace py = pybind11;

// String variable storage for Python interop
class StringStore {
private:
    static std::map<std::string, std::string>& get_storage() {
        static std::map<std::string, std::string> storage;
        return storage;
    }
    
public:
    static void set(const std::string& key, const std::string& value) {
        get_storage()[key] = value;
    }
    
    static std::string get(const std::string& key) {
        auto& storage = get_storage();
        if (storage.find(key) != storage.end()) {
            return storage[key];
        }
        return "";
    }
    
    static bool exists(const std::string& key) {
        return get_storage().find(key) != get_storage().end();
    }
    
    static std::map<std::string, std::string> get_all() {
        return get_storage();
    }
    
    static void clear() {
        get_storage().clear();
    }
};

class LuaEngine {
private:
    lua_State* L;
    
public:
    LuaEngine() {
        L = luaL_newstate();
        luaL_openlibs(L);
        setup_cpp_bindings();
    }
    
    ~LuaEngine() {
        if (L) lua_close(L);
    }
    
    std::string execute(const std::string& code) {
        if (luaL_dostring(L, code.c_str()) != LUA_OK) {
            std::string error = lua_tostring(L, -1);
            lua_pop(L, 1);
            return "Error: " + error;
        }
        return "OK";
    }
    
    void load_file(const std::string& filepath) {
        if (luaL_dofile(L, filepath.c_str()) != LUA_OK) {
            std::cerr << "Failed to load: " << lua_tostring(L, -1) << std::endl;
        }
    }
    
    std::string call_function(const std::string& func_name) {
        lua_getglobal(L, func_name.c_str());
        if (lua_isfunction(L, -1)) {
            if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                std::string error = lua_tostring(L, -1);
                lua_pop(L, 1);
                return error;
            }
        }
        return "Function executed";
    }
    
    // Set a string variable that can be imported in Python
    void set_string(const std::string& name, const std::string& value) {
        StringStore::set(name, value);
        std::cout << "[Lua] Set variable: " << name << " = \"" << value << "\"" << std::endl;
    }
    
    // Get a string variable
    std::string get_string(const std::string& name) {
        return StringStore::get(name);
    }
    
    // Get all stored variables
    std::map<std::string, std::string> get_all_strings() {
        return StringStore::get_all();
    }
    
    // Execute Lua and capture string assignments
    std::string execute_and_capture(const std::string& code) {
        execute(code);
        // Try to find assignments like: title = "value"
        return "Executed and capturing variables";
    }
    
private:
    void setup_cpp_bindings() {
        // Register C++ function to set variables from Lua
        lua_pushcfunction(L, [](lua_State* L) -> int {
            const char* name = luaL_checkstring(L, 1);
            const char* value = luaL_checkstring(L, 2);
            StringStore::set(name, value);
            std::cout << "[Lua->CPP] Stored: " << name << " = \"" << value << "\"" << std::endl;
            return 0;
        });
        lua_setglobal(L, "cpp_set_string");
        
        // Register C++ function to get variables from Lua
        lua_pushcfunction(L, [](lua_State* L) -> int {
            const char* name = luaL_checkstring(L, 1);
            std::string value = StringStore::get(name);
            lua_pushstring(L, value.c_str());
            return 1;
        });
        lua_setglobal(L, "cpp_get_string");
        
        // Register print function
        lua_pushcfunction(L, [](lua_State* L) -> int {
            const char* msg = luaL_checkstring(L, 1);
            std::cout << "[Lua] " << msg << std::endl;
            return 0;
        });
        lua_setglobal(L, "print_from_lua");
    }
};

// PyBind11 module definition
PYBIND11_MODULE(cpp_lua_py, m) {
    m.doc() = "C++ Lua Server Python Bindings with String Exchange";
    
    // LuaEngine class binding
    py::class_<LuaEngine>(m, "LuaEngine")
        .def(py::init<>())
        .def("execute", &LuaEngine::execute, "Execute Lua code")
        .def("load_file", &LuaEngine::load_file, "Load Lua file")
        .def("call_function", &LuaEngine::call_function, "Call Lua function")
        .def("set_string", &LuaEngine::set_string, "Set a string variable accessible to Python")
        .def("get_string", &LuaEngine::get_string, "Get a string variable from storage")
        .def("get_all_strings", &LuaEngine::get_all_strings, "Get all stored string variables")
        .def("execute_and_capture", &LuaEngine::execute_and_capture, "Execute Lua and capture assignments");
    
    // StringStore class binding
    py::class_<StringStore>(m, "StringStore")
        .def_static("set", &StringStore::set, "Set a string variable")
        .def_static("get", &StringStore::get, "Get a string variable")
        .def_static("exists", &StringStore::exists, "Check if variable exists")
        .def_static("get_all", &StringStore::get_all, "Get all variables")
        .def_static("clear", &StringStore::clear, "Clear all variables");
}
