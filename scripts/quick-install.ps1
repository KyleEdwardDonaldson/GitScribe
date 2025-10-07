# GitScribe Quick Install (uses existing debug builds)
#Requires -RunAsAdministrator

$ErrorActionPreference = "Stop"

Write-Host "=== GitScribe Quick Install ===" -ForegroundColor Cyan

# Paths
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$installDir = "$env:ProgramFiles\GitScribe"
$shellDll = Join-Path $scriptDir "gitscribe-shell\build\bin\Release\GitScribeShell.dll"
$coreDll = Join-Path $scriptDir "gitscribe-core\target\x86_64-pc-windows-msvc\debug\gitscribe_core.dll"

# Check if files exist
if (-not (Test-Path $shellDll)) {
    Write-Host "Looking for: $shellDll" -ForegroundColor Red
    throw "GitScribeShell.dll not found. Run CMake build first."
}
if (-not (Test-Path $coreDll)) {
    Write-Host "Looking for: $coreDll" -ForegroundColor Red
    throw "gitscribe_core.dll not found. Build Rust core first."
}

# Step 1: Unregister and cleanup old installation
Write-Host ""
Write-Host "[1/6] Cleaning up previous installation..." -ForegroundColor Yellow
if (Test-Path $installDir) {
    Write-Host "  Unregistering old shell extension..." -ForegroundColor Gray
    $oldDll = "$installDir\bin\GitScribeShell.dll"
    if (Test-Path $oldDll) {
        regsvr32 /u /s $oldDll 2>$null
    }

    Write-Host "  Restarting Explorer to unlock DLLs..." -ForegroundColor Gray
    Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 2

    Write-Host "  Removing old files..." -ForegroundColor Gray
    Remove-Item -Recurse -Force $installDir -ErrorAction SilentlyContinue
}

# Step 2: Create installation directory
Write-Host ""
Write-Host "[2/6] Creating installation directory..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path "$installDir\bin" -Force | Out-Null
New-Item -ItemType Directory -Path "$installDir\resources" -Force | Out-Null

# Step 3: Copy DLLs
Write-Host ""
Write-Host "[3/6] Copying DLLs..." -ForegroundColor Yellow
Copy-Item $shellDll "$installDir\bin\GitScribeShell.dll"
Copy-Item $coreDll "$installDir\bin\gitscribe_core.dll"

# Step 4: Copy icon packs
Write-Host ""
Write-Host "[4/6] Copying icon packs..." -ForegroundColor Yellow
$iconPacksDir = Join-Path $scriptDir "gitscribe-shell\resources\icon-packs"
Copy-Item -Recurse $iconPacksDir "$installDir\resources\"

# Step 5: Register shell extension
Write-Host ""
Write-Host "[5/6] Registering shell extension..." -ForegroundColor Yellow
$dllPath = "$installDir\bin\GitScribeShell.dll"
$result = Start-Process "regsvr32" -ArgumentList "/s", $dllPath -Wait -PassThru
if ($result.ExitCode -ne 0) {
    throw "Failed to register shell extension (error code: $($result.ExitCode))"
}

# Step 6: Restart Explorer
Write-Host ""
Write-Host "[6/6] Restarting Windows Explorer..." -ForegroundColor Yellow
Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2
Start-Process explorer

Write-Host ""
Write-Host "Installation complete!" -ForegroundColor Green
Write-Host ""
Write-Host "GitScribe is installed at: $installDir" -ForegroundColor Cyan
Write-Host "Navigate to a Git repository to see overlay icons!" -ForegroundColor Gray
