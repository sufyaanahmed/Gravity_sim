@echo off
echo Compiling gravity simulation...
g++ -g -std=c++17 -I./include -L./lib src/main.cpp src/glad.c -lglfw3dll -o gravity_sim.exe
if %ERRORLEVEL% == 0 (
    echo Compilation successful! Run with: gravity_sim.exe
) else (
    echo Compilation failed. Make sure g++ is installed and in your PATH.
)
pause 