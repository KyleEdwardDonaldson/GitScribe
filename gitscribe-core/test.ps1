# Test script for gitscribe-core

# Add Cargo to PATH
$env:PATH = "$env:USERPROFILE\.cargo\bin;$env:PATH"

Write-Host "Running tests..." -ForegroundColor Cyan
cargo test

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nAll tests passed!" -ForegroundColor Green
} else {
    Write-Host "`nTests failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}
