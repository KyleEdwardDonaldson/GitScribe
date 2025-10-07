@echo off
REM GitScribe Shell Extension - Unregistration Script
REM Must be run as Administrator

echo GitScribe Shell Extension Unregistration
echo ==========================================
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

echo Unregistering: %DLL_PATH%
echo.

REM Check if DLL exists
if not exist "%DLL_PATH%" (
    echo WARNING: GitScribeShell.dll not found!
    echo Expected location: %DLL_PATH%
    echo.
    echo Attempting to unregister anyway...
)

REM Unregister the DLL
echo Unregistering COM server...
regsvr32 /u /s "%DLL_PATH%"

if %errorLevel% equ 0 (
    echo SUCCESS: GitScribe Shell Extension unregistered!
    echo.
    echo Restarting Windows Explorer...
    taskkill /f /im explorer.exe
    start explorer.exe
    echo.
    echo Unregistration complete!
) else (
    echo ERROR: Unregistration failed!
    echo Error code: %errorLevel%
)

echo.
pause
