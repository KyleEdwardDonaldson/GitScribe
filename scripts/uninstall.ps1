# GitScribe Uninstaller

#Requires -RunAsAdministrator

$ErrorActionPreference = "Stop"

Write-Host "=== GitScribe Uninstaller ===" -ForegroundColor Cyan

$installDir = "$env:ProgramFiles\GitScribe"

if (-not (Test-Path $installDir)) {
    Write-Host "GitScribe is not installed." -ForegroundColor Yellow
    exit 0
}

# Step 1: Unregister shell extension
Write-Host ""
Write-Host "[1/3] Unregistering shell extension..." -ForegroundColor Yellow
$dllPath = "$installDir\bin\GitScribeShell.dll"
if (Test-Path $dllPath) {
    regsvr32 /u /s $dllPath
}

# Step 2: Remove files
Write-Host ""
Write-Host "[2/3] Removing files..." -ForegroundColor Yellow
Remove-Item -Recurse -Force $installDir

# Step 3: Restart Explorer
Write-Host ""
Write-Host "[3/3] Restarting Windows Explorer..." -ForegroundColor Yellow
Stop-Process -Name explorer -Force
Start-Sleep -Seconds 2
Start-Process explorer

Write-Host ""
Write-Host "GitScribe has been uninstalled." -ForegroundColor Green
