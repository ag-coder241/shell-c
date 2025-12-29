A custom shell implementation in C that implements standard shell behaviour with persistent history, 
pipelining, tab completion and general shell commands.

Features:
1. Built-in Commands: cd, pwd, echo, type, history, clear, exit.
2. Pipelining: Support for multiple commands.
3. Redirection: Input/Output redirection.
4. History: Persistent history with append/read/write support.
5. Autocomplete: Tab completion for commands and file paths.

Build & Run - 

Prerequisites
1. GCC Compiler
2. CMake (Version 3.13+)
3. Readline Library

// Create a build directory
mkdir build
cd build

// Generate build files
cmake ..

// Compile
make

// Run the shell
./shell