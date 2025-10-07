@echo off
REM GitScribe Shell Extension - DEBUG Registration Script
REM Must be run as Administrator

echo GitScribe Shell Extension Registration (DEBUG)
echo ================================================
echo.

REM Check for admin rights
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator!
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

REM Get the directory where this script is located
set SCRIPT_DIR=%~dp0
set DLL_PATH=%SCRIPT_DIR%build\bin\Debug\GitScribeShell.dll

echo Registering: %DLL_PATH%
echo.

REM Check if DLL exists
if not exist "%DLL_PATH%" (
    echo ERROR: GitScribeShell.dll not found!
    echo Expected location: %DLL_PATH%
    echo.
    echo Please build the project first:
    echo   cd build
    echo   cmake --build . --config Debug
    pause
    exit /b 1
)

REM Unregister old Release build first
set OLD_DLL=%SCRIPT_DIR%build\bin\Release\GitScribeShell.dll
if exist "%OLD_DLL%" (
    echo Unregistering old Release build...
    regsvr32 /u /s "%OLD_DLL%"
)

REM Kill Explorer to release DLL
echo Killing Explorer...
taskkill /f /im explorer.exe >nul 2>&1

REM Wait a moment
timeout /t 2 /nobreak >nul

REM Register the DEBUG DLL
echo Registering DEBUG COM server...
regsvr32 /s "%DLL_PATH%"

if %errorLevel% equ 0 (
    echo SUCCESS: GitScribe Shell Extension (DEBUG) registered!
    echo.
    echo Starting Windows Explorer...
    start explorer.exe
    echo.
    echo Registration complete!
    echo You should now see GitScribe context menus in Windows Explorer.
    echo.
    echo REMINDER: This is a DEBUG build - use DebugView to see logs!
) else (
    echo ERROR: Registration failed!
    echo Error code: %errorLevel%
    start explorer.exe
)

echo.
pause
