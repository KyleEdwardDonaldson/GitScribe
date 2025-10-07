# Run the status demo
param(
    [string]$Path = "."
)

# Add Cargo to PATH
$env:PATH = "$env:USERPROFILE\.cargo\bin;$env:PATH"

Write-Host "Building and running status demo..." -ForegroundColor Cyan
Write-Host ""

if ($Path -eq ".") {
    cargo run --example status_demo
} else {
    cargo run --example status_demo -- $Path
}
