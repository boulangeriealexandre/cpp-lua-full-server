#!/usr/bin/env python3
"""
Example: Lua Hello World script with Python import
Demonstrates Lua execution and variable import in Python
"""

from cpp_lua_py import LuaEngine, StringStore

def main():
    print("=" * 60)
    print("C++ Lua Server - Hello World Example")
    print("=" * 60)
    
    # Create Lua engine
    print("\n1️⃣ Creating LuaEngine instance...")
    engine = LuaEngine()
    print("✓ LuaEngine created successfully\n")
    
    # Execute Lua code that prints "Hello World"
    print("2️⃣ Executing Lua code: print('Hello from Lua!')")
    result = engine.execute("print('Hello from Lua!')")
    print(f"   Result: {result}\n")
    
    # Set a Lua variable
    print("3️⃣ Setting Lua variable: message = 'Hello, World!'")
    engine.set_string("message", "Hello, World!")
    print("✓ Variable set in Lua\n")
    
    # Import variable in Python
    print("4️⃣ Importing variable from Lua to Python...")
    message = engine.get_string("message")
    print(f"✓ Imported message: '{message}'\n")
    
    # Execute more complex Lua code
    print("5️⃣ Executing complex Lua code...")
    lua_code = """
    greeting = "Hello from Lua"
    name = "World"
    full_message = greeting .. ", " .. name .. "!"
    print("Lua created: " .. full_message)
    cpp_set_string("full_message", full_message)
    """
    engine.execute(lua_code)
    print("✓ Complex Lua code executed\n")
    
    # Import the complex result
    print("6️⃣ Importing complex result from Lua...")
    full_message = StringStore.get("full_message")
    print(f"✓ Imported full message: '{full_message}'\n")
    
    # Multiple variables
    print("7️⃣ Setting multiple variables in Lua...")
    engine.set_string("title", "C++ Lua Server")
    engine.set_string("version", "1.0.0")
    engine.set_string("author", "Alexandre")
    print("✓ Multiple variables set\n")
    
    # Get all variables
    print("8️⃣ Getting all variables from StringStore...")
    all_vars = engine.get_all_strings()
    print("✓ All stored variables:")
    for key, value in all_vars.items():
        print(f"   {key}: '{value}'")
    print()
    
    # Call Lua function
    print("9️⃣ Calling Lua function...")
    lua_func = """
    function greet(name)
        message = "Hello, " .. name .. "!"
        print(message)
        cpp_set_string("greeting_result", message)
        return message
    end
    
    greet("Python")
    """
    engine.execute(lua_func)
    greeting_result = StringStore.get("greeting_result")
    print(f"✓ Function result: '{greeting_result}'\n")
    
    # Complex data structure
    print("🔟 Working with Lua tables and loops...")
    lua_complex = """
    fruits = {}
    fruits[1] = "Apple"
    fruits[2] = "Banana"
    fruits[3] = "Orange"
    
    result = ""
    for i = 1, #fruits do
        result = result .. fruits[i]
        if i < #fruits then
            result = result .. ", "
        end
    end
    
    print("Fruits: " .. result)
    cpp_set_string("fruits_list", result)
    """
    engine.execute(lua_complex)
    fruits = StringStore.get("fruits_list")
    print(f"✓ Lua table result: '{fruits}'\n")
    
    # Summary
    print("=" * 60)
    print("Summary of Imported Variables:")
    print("=" * 60)
    all_vars = engine.get_all_strings()
    for key, value in all_vars.items():
        print(f"  • {key}: {value}")
    print("\n✓ All operations completed successfully!")
    print("=" * 60)

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()
