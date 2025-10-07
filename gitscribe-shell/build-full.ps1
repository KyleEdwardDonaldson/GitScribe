# Build GitScribe Full (with all features)

Write-Host "Building GitScribe Full..." -ForegroundColor Cyan

# Step 1: Build Rust core library
Write-Host "`n[1/3] Building gitscribe-core (Rust library)..." -ForegroundColor Yellow
Push-Location ..\gitscribe-core
cargo build --release
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Rust build failed" -ForegroundColor Red
    Pop-Location
    exit 1
}
Pop-Location

# Step 2: Configure CMake (Full version, no GITSCRIBE_STATUS flag)
Write-Host "`n[2/3] Configuring CMake for GitScribe Full..." -ForegroundColor Yellow
if (Test-Path "build") {
    Remove-Item -Recurse -Force "build"
}
New-Item -ItemType Directory -Path "build" | Out-Null
Push-Location build

cmake .. -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Step 3: Build with MSBuild
Write-Host "`n[3/3] Building shell extension DLL..." -ForegroundColor Yellow
cmake --build . --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

Write-Host "`n==============================" -ForegroundColor Green
Write-Host "GitScribe Full build complete!" -ForegroundColor Green
Write-Host "==============================" -ForegroundColor Green
Write-Host "`nOutput files:"
Write-Host "  DLL: build\bin\Release\GitScribeShell.dll" -ForegroundColor Cyan
Write-Host "  Core: build\bin\Release\gitscribe_core.dll" -ForegroundColor Cyan
Write-Host "`nTo register:"
Write-Host "  cd build\bin\Release" -ForegroundColor Yellow
Write-Host "  regsvr32 GitScribeShell.dll" -ForegroundColor Yellow
