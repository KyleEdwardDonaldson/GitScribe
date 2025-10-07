# Build GitScribe Status v0.1
# This builds the Status version with overlays + simple context menu only

Write-Host "Building GitScribe Status v0.1..." -ForegroundColor Cyan

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

# Step 2: Configure CMake with GITSCRIBE_STATUS option
Write-Host "`n[2/3] Configuring CMake for GitScribe Status..." -ForegroundColor Yellow
if (Test-Path "build-status") {
    Remove-Item -Recurse -Force "build-status"
}
New-Item -ItemType Directory -Path "build-status" | Out-Null
Push-Location build-status

cmake .. -G "Visual Studio 17 2022" -A x64 -DGITSCRIBE_STATUS=ON
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

Write-Host "`n==================================" -ForegroundColor Green
Write-Host "GitScribe Status build complete!" -ForegroundColor Green
Write-Host "==================================" -ForegroundColor Green
Write-Host "`nOutput files:"
Write-Host "  DLL: build-status\bin\Release\GitScribeShell.dll" -ForegroundColor Cyan
Write-Host "  Core: build-status\bin\Release\gitscribe_core.dll" -ForegroundColor Cyan
Write-Host "`nTo register:"
Write-Host "  cd build-status\bin\Release" -ForegroundColor Yellow
Write-Host "  regsvr32 GitScribeShell.dll" -ForegroundColor Yellow
