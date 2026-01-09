@echo off
REM Test script for gempp (Windows)

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..
set BUILD_DIR=%PROJECT_DIR%\build
set TESTS_DIR=%PROJECT_DIR%\tests

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

echo === Running tests ===
set PASSED=0
set FAILED=0

REM Iterate over all test directories
for /d %%D in ("%TESTS_DIR%\*") do (
    call :run_test "%%~nxD"
)

echo.
echo === Results ===
echo Passed: %PASSED%
echo Failed: %FAILED%

if %FAILED% gtr 0 (
    pause
    exit /b 1
)
pause
exit /b 0

:run_test
set TEST_NAME=%~1
set TEST_DIR=%TESTS_DIR%\%TEST_NAME%
set INPUT=%TEST_DIR%\input.txt
set EXPECTED=%TEST_DIR%\expected.txt
set ACTUAL=%TEMP%\gempp_test_output.txt
set ACTUAL_CORE=%TEMP%\gempp_test_actual_core.txt
set EXPECTED_CORE=%TEMP%\gempp_test_expected_core.txt

if not exist "%INPUT%" (
    echo SKIP: %TEST_NAME% ^(missing input.txt^)
    exit /b 0
)
if not exist "%EXPECTED%" (
    echo SKIP: %TEST_NAME% ^(missing expected.txt^)
    exit /b 0
)

"%EXE%" "%INPUT%" > "%ACTUAL%" 2>&1

REM Compare only first 5 lines (GED, Is Subgraph, Minimal Extension, Vertices count, Edges count)
REM The specific vertices/edges can vary between equivalent optimal solutions
for /f "tokens=1,* delims=:" %%a in ('findstr /n "^" "%ACTUAL%"') do (
    if %%a leq 5 echo %%b>> "%ACTUAL_CORE%"
)
for /f "tokens=1,* delims=:" %%a in ('findstr /n "^" "%EXPECTED%"') do (
    if %%a leq 5 echo %%b>> "%EXPECTED_CORE%"
)

fc /w "%EXPECTED_CORE%" "%ACTUAL_CORE%" >nul 2>&1
if errorlevel 1 (
    echo FAIL: %TEST_NAME%
    echo   Expected:
    type "%EXPECTED%"
    echo   Actual:
    type "%ACTUAL%"
    set /a FAILED+=1
) else (
    echo PASS: %TEST_NAME%
    set /a PASSED+=1
)

REM Cleanup temp files
del "%ACTUAL_CORE%" 2>nul
del "%EXPECTED_CORE%" 2>nul

exit /b 0
