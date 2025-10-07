# GitScribe Installer
# Automatically installs shell extension and registers with Windows

#Requires -RunAsAdministrator

$ErrorActionPreference = "Stop"

Write-Host "=== GitScribe Installer ===" -ForegroundColor Cyan

# Installation directory
$installDir = "$env:ProgramFiles\GitScribe"

# Step 1: Create installation directory
Write-Host ""
Write-Host "[1/4] Creating installation directory..." -ForegroundColor Yellow
if (Test-Path $installDir) {
    Write-Host "  Previous installation found, removing..." -ForegroundColor Gray
    # Unregister old version first
    $oldDll = "$installDir\bin\GitScribeShell.dll"
    if (Test-Path $oldDll) {
        regsvr32 /u /s $oldDll 2>$null
    }
    Remove-Item -Recurse -Force $installDir
}
New-Item -ItemType Directory -Path $installDir | Out-Null

# Step 2: Copy files
Write-Host ""
Write-Host "[2/4] Copying files..." -ForegroundColor Yellow
Copy-Item -Recurse "bin" "$installDir\"
Copy-Item -Recurse "resources" "$installDir\"

# Step 3: Register shell extension
Write-Host ""
Write-Host "[3/4] Registering shell extension..." -ForegroundColor Yellow
$dllPath = "$installDir\bin\GitScribeShell.dll"
$result = Start-Process "regsvr32" -ArgumentList "/s", $dllPath -Wait -PassThru
if ($result.ExitCode -ne 0) {
    throw "Failed to register shell extension (error code: $($result.ExitCode))"
}

# Step 4: Restart Explorer
Write-Host ""
Write-Host "[4/4] Restarting Windows Explorer..." -ForegroundColor Yellow
Stop-Process -Name explorer -Force
Start-Sleep -Seconds 2
Start-Process explorer

Write-Host ""
Write-Host "Installation complete!" -ForegroundColor Green
Write-Host ""
Write-Host "GitScribe is now installed at: $installDir" -ForegroundColor Cyan
Write-Host "Shell overlays will appear in Git repositories." -ForegroundColor Gray
Write-Host ""
Write-Host "To uninstall, run: uninstall.ps1" -ForegroundColor Yellow
