@echo off
REM Build script for Windows with Visual Studio
REM Prerequisites: CMake and Visual Studio (cl.exe) must be installed

echo Building gempp-v2...

if not exist build mkdir build
cd build

REM Configure with CMake (auto-detects Visual Studio)
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo CMake configuration failed. Trying older VS version...
    cmake .. -G "Visual Studio 16 2019" -A x64
)
if errorlevel 1 (
    echo CMake failed. Make sure CMake and Visual Studio are installed.
    pause
    exit /b 1
)

REM Build Release configuration
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo.
echo Build successful! Executable: build\Release\gempp-v2.exe
pause
