@echo off
REM Build script for Windows
REM Prerequisites: CMake and a C/C++ compiler (cl.exe)

echo Building gempp-v2...

if not exist build mkdir build
cd build

cmake ..
if errorlevel 1 (
    echo CMake configuration failed.
    pause
    exit /b 1
)

cmake --build . --config Release
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo.
echo Build successful!
pause
