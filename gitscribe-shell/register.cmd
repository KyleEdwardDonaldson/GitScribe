@echo off
REM GitScribe Shell Extension - Registration Script
REM Must be run as Administrator

echo GitScribe Shell Extension Registration
echo =========================================
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
set DLL_PATH=%SCRIPT_DIR%build\bin\Release\GitScribeShell.dll

echo Registering: %DLL_PATH%
echo.

REM Check if DLL exists
if not exist "%DLL_PATH%" (
    echo ERROR: GitScribeShell.dll not found!
    echo Expected location: %DLL_PATH%
    echo.
    echo Please build the project first:
    echo   cd build
    echo   cmake --build . --config Release
    pause
    exit /b 1
)

REM Register the DLL
echo Registering COM server...
regsvr32 /s "%DLL_PATH%"

if %errorLevel% equ 0 (
    echo SUCCESS: GitScribe Shell Extension registered!
    echo.
    echo Restarting Windows Explorer to load the extension...
    taskkill /f /im explorer.exe
    start explorer.exe
    echo.
    echo Registration complete!
    echo You should now see GitScribe context menus in Windows Explorer.
) else (
    echo ERROR: Registration failed!
    echo Error code: %errorLevel%
)

echo.
pause
