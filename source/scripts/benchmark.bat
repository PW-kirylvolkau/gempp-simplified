@echo off
REM Benchmark script for gempp (Windows)
REM Generates benchmark graphs of various sizes and measures computation time

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..
set BUILD_DIR=%PROJECT_DIR%\build
set BENCHMARKS_DIR=%PROJECT_DIR%\benchmarks
set RESULTS_FILE=%BENCHMARKS_DIR%\results.csv

echo === Building gempp ===
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

cmake .. >nul
if errorlevel 1 (
    echo CMake configuration failed
    exit /b 1
)

cmake --build . --config Release >nul
if errorlevel 1 (
    echo Build failed
    exit /b 1
)

cd "%PROJECT_DIR%"

set EXE=%PROJECT_DIR%\gempp.exe

if not exist "%EXE%" (
    echo Build failed: executable not found at %EXE%
    exit /b 1
)

if not exist "%BENCHMARKS_DIR%" mkdir "%BENCHMARKS_DIR%"

echo === Running benchmarks ===
echo Pattern Size,Target Size,Pattern Type,Target Type,Time (ms),GED > "%RESULTS_FILE%"

REM Run benchmarks on test files
echo --- Running on existing tests ---
for /d %%D in ("%PROJECT_DIR%\tests\*") do (
    set TEST_NAME=%%~nxD
    set INPUT=%%D\input.txt
    if exist "!INPUT!" (
        for /f "tokens=*" %%O in ('"%EXE%" --time "!INPUT!" 2^>^&1') do (
            echo %%O | findstr /C:"Time:" >nul && (
                for /f "tokens=2" %%T in ("%%O") do set TIME_MS=%%T
            )
            echo %%O | findstr /C:"GED:" >nul && (
                for /f "tokens=2" %%G in ("%%O") do set GED_VAL=%%G
            )
        )
        echo !TEST_NAME!: !TIME_MS!ms, GED=!GED_VAL!
    )
)

echo.
echo === Benchmark complete ===
echo Results saved to: %RESULTS_FILE%
pause
