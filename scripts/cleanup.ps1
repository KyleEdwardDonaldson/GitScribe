# GitScribe Cleanup - Force remove locked installation
#Requires -RunAsAdministrator

$ErrorActionPreference = "Stop"

Write-Host "=== GitScribe Cleanup ===" -ForegroundColor Cyan

$installDir = "$env:ProgramFiles\GitScribe"

if (-not (Test-Path $installDir)) {
    Write-Host "GitScribe is not installed." -ForegroundColor Yellow
    exit 0
}

Write-Host ""
Write-Host "[1/4] Unregistering shell extension..." -ForegroundColor Yellow
$dllPath = "$installDir\bin\GitScribeShell.dll"
if (Test-Path $dllPath) {
    regsvr32 /u /s $dllPath 2>$null
}

Write-Host ""
Write-Host "[2/4] Killing Explorer to unlock DLLs..." -ForegroundColor Yellow
Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 3

Write-Host ""
Write-Host "[3/4] Removing files..." -ForegroundColor Yellow
$retries = 0
$maxRetries = 5
while ((Test-Path $installDir) -and ($retries -lt $maxRetries)) {
    try {
        Remove-Item -Recurse -Force $installDir -ErrorAction Stop
        break
    } catch {
        $retries++
        Write-Host "  Retry $retries/$maxRetries..." -ForegroundColor Gray
        Start-Sleep -Seconds 2
    }
}

if (Test-Path $installDir) {
    Write-Host "  WARNING: Could not remove all files. Please reboot and try again." -ForegroundColor Yellow
} else {
    Write-Host "  All files removed successfully!" -ForegroundColor Green
}

Write-Host ""
Write-Host "[4/4] Restarting Explorer..." -ForegroundColor Yellow
Start-Process explorer

Write-Host ""
Write-Host "Cleanup complete!" -ForegroundColor Green
Write-Host "You can now run quick-install.ps1" -ForegroundColor Cyan
