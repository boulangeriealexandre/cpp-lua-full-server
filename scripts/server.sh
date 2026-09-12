#!/bin/bash

# C++ Lua Server Control Script
# Manages server start/stop operations with atomic locking

SERVER_NAME="cpp-lua-server"
BUILD_DIR="./build"
EXECUTABLE="${BUILD_DIR}/${SERVER_NAME}"
PID_FILE="/tmp/${SERVER_NAME}.pid"
LOCK_FILE="/tmp/${SERVER_NAME}.lock"
LOG_FILE="/var/log/${SERVER_NAME}.log"
PORT="${1:-8080}"
DB_HOST="${2:-localhost}"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to acquire lock
acquire_lock() {
    local timeout=5
    local elapsed=0
    while [ -f "$LOCK_FILE" ] && [ $elapsed -lt $timeout ]; do
        sleep 0.1
        elapsed=$((elapsed + 1))
    done
    
    if [ -f "$LOCK_FILE" ]; then
        echo -e "${RED}✗ Failed to acquire lock. Server may be in an unstable state.${NC}"
        return 1
    fi
    
    touch "$LOCK_FILE"
    return 0
}

# Function to release lock
release_lock() {
    rm -f "$LOCK_FILE"
}

# Function to start the server
start_server() {
    if ! acquire_lock; then
        return 1
    fi
    
    trap "release_lock" EXIT
    
    # Check if already running
    if [ -f "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if kill -0 "$pid" 2>/dev/null; then
            echo -e "${YELLOW}⚠ Server is already running (PID: $pid)${NC}"
            return 0
        else
            echo -e "${YELLOW}⚠ Removing stale PID file${NC}"
            rm -f "$PID_FILE"
        fi
    fi
    
    # Check if executable exists
    if [ ! -f "$EXECUTABLE" ]; then
        echo -e "${RED}✗ Executable not found: $EXECUTABLE${NC}"
        echo -e "${YELLOW}  Please build the project first: mkdir build && cd build && cmake .. && make${NC}"
        return 1
    fi
    
    # Start the server
    echo -e "${YELLOW}→ Starting C++ Lua Server on port $PORT...${NC}"
    
    mkdir -p "$(dirname "$LOG_FILE")"
    nohup "$EXECUTABLE" "$PORT" "$DB_HOST" >> "$LOG_FILE" 2>&1 &
    local server_pid=$!
    
    # Wait a moment for server to start
    sleep 1
    
    # Check if process is still running
    if kill -0 "$server_pid" 2>/dev/null; then
        echo "$server_pid" > "$PID_FILE"
        echo -e "${GREEN}✓ Server started successfully (PID: $server_pid)${NC}"
        echo -e "${GREEN}  Listening on port $PORT${NC}"
        echo -e "${GREEN}  Database host: $DB_HOST${NC}"
        echo -e "${GREEN}  Log file: $LOG_FILE${NC}"
        return 0
    else
        echo -e "${RED}✗ Server failed to start. Check log: $LOG_FILE${NC}"
        tail -n 10 "$LOG_FILE"
        return 1
    fi
}

# Function to stop the server
stop_server() {
    if ! acquire_lock; then
        return 1
    fi
    
    trap "release_lock" EXIT
    
    # Check if PID file exists
    if [ ! -f "$PID_FILE" ]; then
        echo -e "${YELLOW}⚠ Server is not running (no PID file)${NC}"
        return 0
    fi
    
    local pid=$(cat "$PID_FILE")
    
    # Check if process is running
    if ! kill -0 "$pid" 2>/dev/null; then
        echo -e "${YELLOW}⚠ Server is not running (PID $pid not found)${NC}"
        rm -f "$PID_FILE"
        return 0
    fi
    
    # Send SIGTERM
    echo -e "${YELLOW}→ Stopping server (PID: $pid)...${NC}"
    kill -TERM "$pid" 2>/dev/null
    
    # Wait for graceful shutdown
    local timeout=10
    local elapsed=0
    while [ $elapsed -lt $timeout ]; do
        if ! kill -0 "$pid" 2>/dev/null; then
            rm -f "$PID_FILE"
            echo -e "${GREEN}✓ Server stopped successfully${NC}"
            echo -e "${GREEN}  Server OFF${NC}"
            return 0
        fi
        sleep 1
        elapsed=$((elapsed + 1))
    done
    
    # Force kill if graceful shutdown failed
    echo -e "${YELLOW}⚠ Graceful shutdown timeout. Force killing...${NC}"
    kill -KILL "$pid" 2>/dev/null
    sleep 1
    rm -f "$PID_FILE"
    echo -e "${GREEN}✓ Server force stopped${NC}"
    echo -e "${GREEN}  Server OFF${NC}"
    return 0
}

# Function to check status
status_server() {
    if [ -f "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if kill -0 "$pid" 2>/dev/null; then
            echo -e "${GREEN}✓ Server is running (PID: $pid)${NC}"
            echo -e "${GREEN}  Status: ON${NC}"
            return 0
        else
            echo -e "${RED}✗ Server is not running (stale PID file)${NC}"
            rm -f "$PID_FILE"
            echo -e "${RED}  Status: OFF${NC}"
            return 1
        fi
    else
        echo -e "${RED}✗ Server is not running${NC}"
        echo -e "${RED}  Status: OFF${NC}"
        return 1
    fi
}

# Function to show logs
show_logs() {
    if [ ! -f "$LOG_FILE" ]; then
        echo -e "${YELLOW}⚠ Log file not found: $LOG_FILE${NC}"
        return 1
    fi
    tail -f "$LOG_FILE"
}

# Function to restart
restart_server() {
    echo -e "${YELLOW}→ Restarting server...${NC}"
    stop_server
    sleep 2
    start_server
}

# Main command dispatcher
case "${1:-help}" in
    start)
        start_server
        ;;
    stop)
        stop_server
        ;;
    restart)
        restart_server
        ;;
    status)
        status_server
        ;;
    logs)
        show_logs
        ;;
    *)
        echo "C++ Lua Server Control Script"
        echo ""
        echo "Usage: $0 {start|stop|restart|status|logs} [PORT] [DB_HOST]"
        echo ""
        echo "Commands:"
        echo "  start [PORT] [DB_HOST]  - Start the server (default: port 8080, host localhost)"
        echo "  stop                     - Stop the server (prints 'Server OFF' on success)"
        echo "  restart [PORT] [DB_HOST] - Restart the server"
        echo "  status                   - Show server status"
        echo "  logs                     - Follow server logs"
        echo ""
        echo "Examples:"
        echo "  $0 start"
        echo "  $0 start 9000 mariadb"
        echo "  $0 stop"
        echo "  $0 status"
        echo "  $0 logs"
        ;;
esac
