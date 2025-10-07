# Build script for gitscribe-core with proper MSVC environment setup

Write-Host "Setting up Visual Studio 2022 environment..." -ForegroundColor Cyan

# VS Installation path
$vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Professional"

# MSVC version
$msvcVersion = "14.41.34120"

# Windows SDK version
$sdkVersion = "10.0.22621.0"

# Set up paths
$env:VSINSTALLDIR = "$vsPath\"
$env:VCINSTALLDIR = "$vsPath\VC\"
$env:VCToolsInstallDir = "$vsPath\VC\Tools\MSVC\$msvcVersion\"

# Add MSVC tools to PATH
$env:PATH = "$vsPath\VC\Tools\MSVC\$msvcVersion\bin\Hostx64\x64;$env:PATH"

# Add Windows SDK to PATH
$env:PATH = "C:\Program Files (x86)\Windows Kits\10\bin\$sdkVersion\x64;$env:PATH"

# Add Cargo to PATH
$env:PATH = "$env:USERPROFILE\.cargo\bin;$env:PATH"

# Set include and lib paths for MSVC
$env:INCLUDE = "$vsPath\VC\Tools\MSVC\$msvcVersion\include;$vsPath\VC\Tools\MSVC\$msvcVersion\crt\src;C:\Program Files (x86)\Windows Kits\10\Include\$sdkVersion\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\$sdkVersion\um;C:\Program Files (x86)\Windows Kits\10\Include\$sdkVersion\shared"

$env:LIB = "$vsPath\VC\Tools\MSVC\$msvcVersion\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\$sdkVersion\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\$sdkVersion\um\x64"

Write-Host "Environment configured successfully!" -ForegroundColor Green
Write-Host "MSVC Compiler: " -NoNewline
& cl.exe 2>&1 | Select-String "Version" | Select-Object -First 1
Write-Host ""

Write-Host "Building gitscribe-core..." -ForegroundColor Cyan
cargo build

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "Build succeeded!" -ForegroundColor Green
    Write-Host "Output: target\debug\gitscribe_core.dll" -ForegroundColor Yellow
} else {
    Write-Host ""
    Write-Host "Build failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}
