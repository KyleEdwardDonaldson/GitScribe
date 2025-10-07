@echo off
REM Build script for gitscribe-core

REM Initialize VS environment
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"

REM Add Cargo to PATH
set PATH=%USERPROFILE%\.cargo\bin;%PATH%

REM Build
cargo build %*
