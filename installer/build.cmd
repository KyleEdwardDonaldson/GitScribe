@echo off
REM GitScribe Status - WiX Installer Build Script
REM Usage: build.cmd [Debug|Release]

setlocal enabledelayedexpansion

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

echo ========================================
echo GitScribe Status Installer Build
echo ========================================
echo Configuration: %CONFIG%
echo.

REM Check for WiX Toolset
where candle.exe >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: WiX Toolset not found in PATH
    echo.
    echo Please install WiX Toolset v3.11 or later from:
    echo https://wixtoolset.org/releases/
    echo.
    echo Or install via winget:
    echo   winget install WixToolset.WixToolset
    echo.
    pause
    exit /b 1
)

REM Set paths
set ROOT_DIR=%~dp0..
set BIN_DIR=%ROOT_DIR%\gitscribe-shell\build\bin\%CONFIG%
set ICONS_DIR=%ROOT_DIR%\gitscribe-shell\resources\icons
set OUT_DIR=%~dp0output

REM Verify binaries exist
if not exist "%BIN_DIR%\GitScribeShell.dll" (
    echo ERROR: GitScribeShell.dll not found at %BIN_DIR%
    echo.
    echo Please build the shell extension first:
    echo   cd gitscribe-shell\build
    echo   cmake --build . --config %CONFIG%
    echo.
    pause
    exit /b 1
)

if not exist "%BIN_DIR%\gitscribe_core.dll" (
    echo ERROR: gitscribe_core.dll not found at %BIN_DIR%
    echo.
    echo Please build the core library first:
    echo   cd gitscribe-core
    echo   cargo build --release
    echo   copy target\release\gitscribe_core.dll ..\gitscribe-shell\build\bin\%CONFIG%\
    echo.
    pause
    exit /b 1
)

REM Create output directory
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

echo Step 1: Compiling WiX source...
candle.exe -nologo ^
    -dBinDir="%BIN_DIR%" ^
    -dIconsDir="%ICONS_DIR%" ^
    -dConfiguration=%CONFIG% ^
    -arch x64 ^
    -out "%OUT_DIR%\Product.wixobj" ^
    Product.wxs

if %ERRORLEVEL% neq 0 (
    echo ERROR: Candle compilation failed
    pause
    exit /b 1
)

echo Step 2: Linking installer...
light.exe -nologo ^
    -ext WixUIExtension ^
    -cultures:en-US ^
    -out "%OUT_DIR%\GitScribe-Status-Setup.msi" ^
    "%OUT_DIR%\Product.wixobj"

if %ERRORLEVEL% neq 0 (
    echo ERROR: Light linking failed
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Installer: %OUT_DIR%\GitScribe-Status-Setup.msi
echo Size:
for %%A in ("%OUT_DIR%\GitScribe-Status-Setup.msi") do echo   %%~zA bytes (%%~zA / 1024 / 1024 MB)
echo.

REM Optional: Create bundle (MSI + VC++ Redistributable)
set /p CREATE_BUNDLE=Create bundle with VC++ Redistributable? (y/n):
if /i "%CREATE_BUNDLE%"=="y" (
    echo.
    echo Creating bundle...
    REM TODO: Implement bundle creation with Burn
    echo Bundle creation not yet implemented.
    echo Users will need to install VC++ Redistributable separately if not present.
)

echo.
echo To test installation:
echo   msiexec /i "%OUT_DIR%\GitScribe-Status-Setup.msi" /l*v install.log
echo.
echo To uninstall:
echo   msiexec /x "%OUT_DIR%\GitScribe-Status-Setup.msi"
echo.

endlocal
