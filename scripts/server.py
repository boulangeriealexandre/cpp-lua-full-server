#!/usr/bin/env python3
"""
C++ Lua Server Control Script (Python)
Manages server start/stop operations with process management
"""

import os
import sys
import subprocess
import time
import signal
import json
from pathlib import Path
from typing import Optional, Dict, Tuple
import psutil
import argparse
from datetime import datetime

class ServerManager:
    def __init__(self, config_file: str = "server_config.json"):
        self.config_file = config_file
        self.config = self.load_config()
        self.pid_file = self.config.get("pid_file", "/tmp/cpp-lua-server.pid")
        self.log_file = self.config.get("log_file", "/var/log/cpp-lua-server.log")
        self.build_dir = self.config.get("build_dir", "./build")
        self.executable = os.path.join(self.build_dir, "cpp-lua-server")
        self.port = self.config.get("port", 8080)
        self.db_host = self.config.get("db_host", "localhost")
        
    def load_config(self) -> Dict:
        """Load configuration from JSON file or return defaults"""
        if os.path.exists(self.config_file):
            try:
                with open(self.config_file, 'r') as f:
                    return json.load(f)
            except json.JSONDecodeError:
                print(f"⚠ Invalid JSON in {self.config_file}, using defaults")
        return {}
    
    def save_config(self):
        """Save current configuration to JSON file"""
        config = {
            "port": self.port,
            "db_host": self.db_host,
            "build_dir": self.build_dir,
            "pid_file": self.pid_file,
            "log_file": self.log_file,
        }
        try:
            os.makedirs(os.path.dirname(self.config_file) or ".", exist_ok=True)
            with open(self.config_file, 'w') as f:
                json.dump(config, f, indent=2)
            print(f"✓ Configuration saved to {self.config_file}")
        except Exception as e:
            print(f"✗ Failed to save config: {e}")
    
    def log_message(self, message: str, level: str = "INFO"):
        """Log message to file and console"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_entry = f"[{timestamp}] [{level}] {message}"
        
        print(log_entry)
        
        try:
            os.makedirs(os.path.dirname(self.log_file), exist_ok=True)
            with open(self.log_file, 'a') as f:
                f.write(log_entry + "\n")
        except Exception as e:
            print(f"⚠ Failed to write to log: {e}")
    
    def is_running(self) -> Tuple[bool, Optional[int]]:
        """Check if server is running and return PID"""
        if not os.path.exists(self.pid_file):
            return False, None
        
        try:
            with open(self.pid_file, 'r') as f:
                pid = int(f.read().strip())
            
            # Check if process exists
            if psutil.pid_exists(pid):
                process = psutil.Process(pid)
                if process.name() in ["cpp-lua-server", "python"]:
                    return True, pid
            
            # PID file is stale
            os.remove(self.pid_file)
            return False, None
        except (ValueError, FileNotFoundError, psutil.NoSuchProcess):
            return False, None
    
    def start_server(self, port: Optional[int] = None, db_host: Optional[str] = None) -> bool:
        """Start the server"""
        if port:
            self.port = port
        if db_host:
            self.db_host = db_host
        
        # Check if already running
        running, pid = self.is_running()
        if running:
            print(f"⚠ Server is already running (PID: {pid})")
            return True
        
        # Check if executable exists
        if not os.path.exists(self.executable):
            print(f"✗ Executable not found: {self.executable}")
            print(f"  Please build the project first:")
            print(f"  mkdir build && cd build && cmake .. && make")
            return False
        
        try:
            print(f"→ Starting C++ Lua Server on port {self.port}...")
            
            # Create log file directory
            os.makedirs(os.path.dirname(self.log_file), exist_ok=True)
            
            # Start the server process
            with open(self.log_file, 'a') as log:
                process = subprocess.Popen(
                    [self.executable, str(self.port), self.db_host],
                    stdout=log,
                    stderr=subprocess.STDOUT,
                    preexec_fn=os.setsid  # Create new process group
                )
            
            # Wait a moment for server to start
            time.sleep(1)
            
            # Check if process is still running
            if process.poll() is not None:
                print(f"✗ Server failed to start. Check log: {self.log_file}")
                with open(self.log_file, 'r') as f:
                    print(f.read()[-500:])  # Print last 500 chars
                return False
            
            # Save PID
            with open(self.pid_file, 'w') as f:
                f.write(str(process.pid))
            
            print(f"✓ Server started successfully (PID: {process.pid})")
            print(f"  Listening on port {self.port}")
            print(f"  Database host: {self.db_host}")
            print(f"  Log file: {self.log_file}")
            
            self.log_message(f"Server started on port {self.port} with DB host {self.db_host}", "START")
            return True
            
        except Exception as e:
            print(f"✗ Failed to start server: {e}")
            self.log_message(f"Failed to start server: {e}", "ERROR")
            return False
    
    def stop_server(self, force: bool = False) -> bool:
        """Stop the server"""
        running, pid = self.is_running()
        
        if not running:
            print("⚠ Server is not running")
            return True
        
        try:
            print(f"→ Stopping server (PID: {pid})...")
            process = psutil.Process(pid)
            
            if not force:
                # Graceful shutdown with SIGTERM
                process.terminate()
                
                # Wait up to 10 seconds
                try:
                    process.wait(timeout=10)
                    print("✓ Server stopped successfully")
                    print("  Server OFF")
                except psutil.TimeoutExpired:
                    print("⚠ Graceful shutdown timeout. Force killing...")
                    process.kill()
                    print("✓ Server force stopped")
                    print("  Server OFF")
            else:
                # Force kill
                process.kill()
                print("✓ Server force stopped")
                print("  Server OFF")
            
            # Remove PID file
            if os.path.exists(self.pid_file):
                os.remove(self.pid_file)
            
            self.log_message("Server stopped", "STOP")
            return True
            
        except (psutil.NoSuchProcess, ProcessLookupError):
            print("⚠ Server is not running")
            if os.path.exists(self.pid_file):
                os.remove(self.pid_file)
            return True
        except Exception as e:
            print(f"✗ Failed to stop server: {e}")
            self.log_message(f"Failed to stop server: {e}", "ERROR")
            return False
    
    def restart_server(self, port: Optional[int] = None, db_host: Optional[str] = None) -> bool:
        """Restart the server"""
        print("→ Restarting server...")
        if not self.stop_server():
            return False
        time.sleep(2)
        return self.start_server(port, db_host)
    
    def status_server(self) -> bool:
        """Check server status"""
        running, pid = self.is_running()
        
        if running:
            try:
                process = psutil.Process(pid)
                cpu_percent = process.cpu_percent(interval=0.1)
                memory_info = process.memory_info()
                memory_mb = memory_info.rss / (1024 * 1024)
                
                print(f"✓ Server is running (PID: {pid})")
                print(f"  Status: ON")
                print(f"  CPU: {cpu_percent:.1f}%")
                print(f"  Memory: {memory_mb:.1f} MB")
                print(f"  Port: {self.port}")
                print(f"  DB Host: {self.db_host}")
                return True
            except psutil.NoSuchProcess:
                print(f"✗ Server is not running (stale PID file)")
                os.remove(self.pid_file)
                print(f"  Status: OFF")
                return False
        else:
            print("✗ Server is not running")
            print("  Status: OFF")
            return False
    
    def view_logs(self, lines: int = 50, follow: bool = False):
        """View server logs"""
        if not os.path.exists(self.log_file):
            print(f"⚠ Log file not found: {self.log_file}")
            return
        
        if follow:
            print(f"Following logs from {self.log_file} (press Ctrl+C to stop)...")
            try:
                with open(self.log_file, 'r') as f:
                    f.seek(0, 2)  # Go to end
                    while True:
                        line = f.readline()
                        if line:
                            print(line.rstrip())
                        else:
                            time.sleep(0.1)
            except KeyboardInterrupt:
                print("\nStopped following logs")
        else:
            print(f"Last {lines} lines from {self.log_file}:")
            try:
                with open(self.log_file, 'r') as f:
                    all_lines = f.readlines()
                    for line in all_lines[-lines:]:
                        print(line.rstrip())
            except Exception as e:
                print(f"✗ Failed to read logs: {e}")
    
    def build_server(self) -> bool:
        """Build the server from source"""
        print("→ Building C++ Lua Server...")
        
        # Create build directory
        os.makedirs(self.build_dir, exist_ok=True)
        
        try:
            # Run CMake
            print("  Running CMake...")
            result = subprocess.run(
                ["cmake", ".."],
                cwd=self.build_dir,
                capture_output=True,
                text=True
            )
            if result.returncode != 0:
                print(f"✗ CMake failed: {result.stderr}")
                return False
            
            # Run Make
            print("  Running make...")
            result = subprocess.run(
                ["make"],
                cwd=self.build_dir,
                capture_output=True,
                text=True
            )
            if result.returncode != 0:
                print(f"✗ Make failed: {result.stderr}")
                return False
            
            print(f"✓ Build successful")
            self.log_message("Build successful", "BUILD")
            return True
            
        except Exception as e:
            print(f"✗ Build failed: {e}")
            self.log_message(f"Build failed: {e}", "ERROR")
            return False


def main():
    parser = argparse.ArgumentParser(
        description='C++ Lua Server Control Script (Python)',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s start
  %(prog)s start --port 9000 --host mariadb
  %(prog)s stop
  %(prog)s restart
  %(prog)s status
  %(prog)s logs
  %(prog)s logs --follow
  %(prog)s build
  %(prog)s config
        '''
    )
    
    parser.add_argument('command', nargs='?', default='help',
                        choices=['start', 'stop', 'restart', 'status', 'logs', 'build', 'config', 'help'],
                        help='Command to execute')
    parser.add_argument('--port', type=int, help='Server port (default: 8080)')
    parser.add_argument('--host', type=str, help='Database host (default: localhost)')
    parser.add_argument('--config', type=str, default='server_config.json',
                        help='Configuration file (default: server_config.json)')
    parser.add_argument('--follow', action='store_true',
                        help='Follow logs in real-time')
    parser.add_argument('--lines', type=int, default=50,
                        help='Number of log lines to display (default: 50)')
    parser.add_argument('--force', action='store_true',
                        help='Force stop the server')
    
    args = parser.parse_args()
    
    # Create server manager
    manager = ServerManager(args.config)
    
    # Handle commands
    if args.command == 'start':
        success = manager.start_server(args.port, args.host)
        sys.exit(0 if success else 1)
    
    elif args.command == 'stop':
        success = manager.stop_server(args.force)
        sys.exit(0 if success else 1)
    
    elif args.command == 'restart':
        success = manager.restart_server(args.port, args.host)
        sys.exit(0 if success else 1)
    
    elif args.command == 'status':
        manager.status_server()
    
    elif args.command == 'logs':
        manager.view_logs(args.lines, args.follow)
    
    elif args.command == 'build':
        success = manager.build_server()
        sys.exit(0 if success else 1)
    
    elif args.command == 'config':
        print("Current Configuration:")
        print(f"  Port: {manager.port}")
        print(f"  Database Host: {manager.db_host}")
        print(f"  Build Directory: {manager.build_dir}")
        print(f"  Executable: {manager.executable}")
        print(f"  PID File: {manager.pid_file}")
        print(f"  Log File: {manager.log_file}")
        if args.port or args.host:
            if args.port:
                manager.port = args.port
            if args.host:
                manager.db_host = args.host
            manager.save_config()
    
    else:  # help
        parser.print_help()


if __name__ == '__main__':
    main()
