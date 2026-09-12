-- C++ Lua Server - Hello World Script
-- This script demonstrates Lua functionality

print("========================================")
print("Lua Hello World Script Loaded!")
print("========================================")

-- Simple variables
message = "Hello, World!"
print("Message: " .. message)

-- Function definition
function greet(name)
    return "Hello, " .. name .. "!"
end

-- Call function
greeting = greet("Lua Developer")
print(greeting)

-- Table (array) operations
fruits = {"Apple", "Banana", "Orange", "Mango", "Grape"}
print("\nFruits List:")
for i = 1, #fruits do
    print("  " .. i .. ". " .. fruits[i])
end

-- String concatenation
fullName = "C++ Lua Server"
version = "1.0.0"
info = fullName .. " v" .. version
print("\nInfo: " .. info)

-- Conditional logic
function checkNumber(num)
    if num > 0 then
        return "Positive"
    elseif num < 0 then
        return "Negative"
    else
        return "Zero"
    end
end

testNum = 42
result = checkNumber(testNum)
print("Number " .. testNum .. " is: " .. result)

-- Table with key-value pairs (dictionary)
server_config = {
    port = 8080,
    host = "localhost",
    database = "app_db",
    timeout = 30
}

print("\nServer Configuration:")
for key, value in pairs(server_config) do
    print("  " .. key .. ": " .. tostring(value))
end

-- Loop with calculation
print("\nCalculations:")
sum = 0
for i = 1, 10 do
    sum = sum + i
end
print("Sum of 1 to 10: " .. sum)

-- Function with multiple returns
function getDimensions()
    return 800, 600
end

width, height = getDimensions()
print("Screen dimensions: " .. width .. "x" .. height)

-- String operations
text = "Hello from Lua"
print("\nString operations:")
print("  Original: " .. text)
print("  Length: " .. string.len(text))
print("  Uppercase: " .. string.upper(text))
print("  Substring: " .. string.sub(text, 1, 5))

-- Math operations
print("\nMath operations:")
print("  Square root of 16: " .. math.sqrt(16))
print("  2 to the power of 3: " .. math.pow(2, 3))
print("  Pi: " .. math.pi)
print("  Floor of 3.7: " .. math.floor(3.7))

-- Final message
print("\n========================================")
print("Lua Hello World Script Completed!")
print("========================================")

-- Store results for Python import
title = "C++ Lua Server"
status = "Running"
language = "Lua 5.3"
