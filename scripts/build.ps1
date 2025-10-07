# GitScribe Build Script
# Builds Rust core + C++ shell extension + creates installer

param(
    [string]$Config = "Release",
    [switch]$SkipTests
)

$ErrorActionPreference = "Stop"

Write-Host "=== GitScribe Build Script ===" -ForegroundColor Cyan
Write-Host "Configuration: $Config" -ForegroundColor Green

# Step 1: Build Rust core
Write-Host "`n[1/4] Building Rust core..." -ForegroundColor Yellow
Push-Location gitscribe-core
cargo build --release
if ($LASTEXITCODE -ne 0) {
    Pop-Location
    throw "Rust build failed"
}

if (-not $SkipTests) {
    Write-Host "Running Rust tests..." -ForegroundColor Yellow
    cargo test --release
    if ($LASTEXITCODE -ne 0) {
        Pop-Location
        throw "Rust tests failed"
    }
}
Pop-Location

# Step 2: Configure CMake for shell extension
Write-Host "`n[2/4] Configuring shell extension..." -ForegroundColor Yellow
$buildDir = "gitscribe-shell\build"
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Push-Location gitscribe-shell\build
& "C:\Program Files\CMake\bin\cmake.exe" .. -DCMAKE_BUILD_TYPE=$Config
if ($LASTEXITCODE -ne 0) {
    Pop-Location
    throw "CMake configuration failed"
}

# Step 3: Build shell extension
Write-Host "`n[3/4] Building shell extension..." -ForegroundColor Yellow
& "C:\Program Files\CMake\bin\cmake.exe" --build . --config $Config
if ($LASTEXITCODE -ne 0) {
    Pop-Location
    throw "Shell extension build failed"
}
Pop-Location

# Step 4: Copy all dependencies to output directory
Write-Host "`n[4/4] Preparing distribution package..." -ForegroundColor Yellow
$distDir = "dist\GitScribe"
if (Test-Path dist) {
    Remove-Item -Recurse -Force dist
}
New-Item -ItemType Directory -Path $distDir\bin | Out-Null
New-Item -ItemType Directory -Path $distDir\resources | Out-Null

# Copy DLLs
Copy-Item "gitscribe-shell\build\bin\$Config\GitScribeShell.dll" "$distDir\bin\"
Copy-Item "gitscribe-core\target\release\gitscribe_core.dll" "$distDir\bin\"

# Copy icon packs
Copy-Item -Recurse "gitscribe-shell\resources\icon-packs" "$distDir\resources\"

# Copy installer script
Copy-Item "install.ps1" "$distDir\"
Copy-Item "uninstall.ps1" "$distDir\"

Write-Host ""
Write-Host "Build complete!" -ForegroundColor Green
Write-Host "Distribution package: dist\GitScribe" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. cd dist\GitScribe" -ForegroundColor Gray
Write-Host "  2. Run install.ps1 as Administrator" -ForegroundColor Gray
