@echo off
REM Build script for gempp (Windows)
REM Prerequisites: CMake and a C/C++ compiler (cl.exe)

echo === Building gempp ===

set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..
set BUILD_DIR=%PROJECT_DIR%\build

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

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
echo === Build successful ===
echo Executable: %PROJECT_DIR%\gempp.exe
pause
