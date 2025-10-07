@echo off
echo Rebuilding GitScribe Shell Extension
echo =====================================
echo.

cd /d C:\r\GitScribe\gitscribe-shell\build
cmake --build . --config Release

if %errorLevel% equ 0 (
    echo.
    echo Build successful! Registering...
    echo.

    regsvr32 /s bin\Release\GitScribeShell.dll

    if %errorLevel% equ 0 (
        echo Registration successful!
        echo Starting Explorer...
        start explorer.exe

        echo.
        echo All done! Right-click a file to test.
    ) else (
        echo Registration failed!
    )
) else (
    echo Build failed!
)

pause
