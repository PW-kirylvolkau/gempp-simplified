@echo off
REM Benchmark script for gempp (Windows) - GED mode
REM Usage: scripts\benchmark_ged.bat [upper_bound]
REM upper_bound is the pruning parameter in (0,1]; defaults to 1.0

setlocal enabledelayedexpansion

set UP=1
if not "%~1"=="" set UP=%~1

REM Basic validation (best-effort)
for /f "tokens=1-2 delims=." %%A in ("%UP%") do (
    if "%%A"=="0" if "%%B"=="" (
        echo Error: upper bound must be in (0,1], got %UP%
        exit /b 1
    )
)

set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..
set BUILD_DIR=%PROJECT_DIR%\build
set BENCHMARKS_DIR=%PROJECT_DIR%\benchmarks
set RESULTS_FILE=%BENCHMARKS_DIR%\results_ged.csv
set EXE=%PROJECT_DIR%\gempp.exe

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

if not exist "%EXE%" (
    echo Build failed: executable not found at %EXE%
    exit /b 1
)

if not exist "%BENCHMARKS_DIR%" mkdir "%BENCHMARKS_DIR%"

echo === Running GED benchmarks (up=%UP%) ===
echo Pattern Size,Target Size,Pattern Type,Target Type,Time (ms),GED > "%RESULTS_FILE%"

REM Run benchmarks on existing tests in GED mode
echo --- Running on existing tests ---
for /d %%D in ("%PROJECT_DIR%\tests\*") do (
    set TEST_NAME=%%~nxD
    set INPUT=%%D\input.txt
    if exist "!INPUT!" (
        set TIME_MS=
        set GED_VAL=
        for /f "tokens=*" %%O in ('"%EXE%" --ged --up %UP% --time "!INPUT!" 2^>^&1') do (
            echo %%O | findstr /C:"Time:" >nul && (
                for /f "tokens=2" %%T in ("%%O") do set TIME_MS=%%T
            )
            echo %%O | findstr /C:"GED:" >nul && (
                for /f "tokens=2" %%G in ("%%O") do set GED_VAL=%%G
            )
        )
        echo !TEST_NAME!: !TIME_MS!ms, GED=!GED_VAL!
        echo ,,,test:!TEST_NAME!,!TIME_MS!,!GED_VAL! >> "%RESULTS_FILE%"
    )
)

echo.
echo === GED benchmark complete ===
echo Results saved to: %RESULTS_FILE%
pause

