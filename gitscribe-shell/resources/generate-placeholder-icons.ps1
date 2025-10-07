# Generate placeholder icons for GitScribe shell extension
# Creates simple colored circles as .ico files

Add-Type -AssemblyName System.Drawing

$icons = @(
    @{ Name = "modified"; Color = [System.Drawing.Color]::FromArgb(244, 196, 48); Symbol = "●" }      # Yellow
    @{ Name = "clean"; Color = [System.Drawing.Color]::FromArgb(28, 228, 212); Symbol = "✓" }         # Turquoise
    @{ Name = "added"; Color = [System.Drawing.Color]::FromArgb(0, 168, 107); Symbol = "+" }          # Green
    @{ Name = "untracked"; Color = [System.Drawing.Color]::FromArgb(153, 102, 204); Symbol = "?" }    # Purple
    @{ Name = "conflicted"; Color = [System.Drawing.Color]::FromArgb(227, 66, 52); Symbol = "!" }     # Red
    @{ Name = "ignored"; Color = [System.Drawing.Color]::FromArgb(120, 113, 108); Symbol = "×" }      # Gray
)

$sizes = @(16, 32, 48, 128)

Write-Host "Generating placeholder icons..." -ForegroundColor Cyan

foreach ($icon in $icons) {
    $name = $icon.Name
    $color = $icon.Color
    $symbol = $icon.Symbol

    Write-Host "  Creating $name.ico ($symbol)" -ForegroundColor Yellow

    # Create PNGs for each size
    $pngFiles = @()
    foreach ($size in $sizes) {
        $bitmap = New-Object System.Drawing.Bitmap($size, $size)
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)

        # Enable antialiasing
        $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
        $graphics.Clear([System.Drawing.Color]::Transparent)

        # Draw filled circle
        $brush = New-Object System.Drawing.SolidBrush($color)
        $padding = [Math]::Max(1, $size / 8)
        $graphics.FillEllipse($brush, $padding, $padding, $size - ($padding * 2), $size - ($padding * 2))

        # Draw symbol in white
        if ($size -ge 32) {
            $fontSize = [Math]::Max(8, $size * 0.5)
            $font = New-Object System.Drawing.Font("Segoe UI", $fontSize, [System.Drawing.FontStyle]::Bold)
            $textBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)

            $format = New-Object System.Drawing.StringFormat
            $format.Alignment = [System.Drawing.StringAlignment]::Center
            $format.LineAlignment = [System.Drawing.StringAlignment]::Center

            $rect = New-Object System.Drawing.RectangleF(0, 0, $size, $size)
            $graphics.DrawString($symbol, $font, $textBrush, $rect, $format)
        }

        # Save PNG
        $pngPath = ".\$name-$size.png"
        $bitmap.Save($pngPath, [System.Drawing.Imaging.ImageFormat]::Png)
        $pngFiles += $pngPath

        $graphics.Dispose()
        $bitmap.Dispose()
    }

    # Check if ImageMagick is available
    $magickPath = Get-Command "magick" -ErrorAction SilentlyContinue

    if ($magickPath) {
        # Convert PNGs to single ICO file using ImageMagick
        $icoPath = ".\$name.ico"
        & magick convert $pngFiles $icoPath

        if ($LASTEXITCODE -eq 0) {
            Write-Host "    ✓ Created $name.ico" -ForegroundColor Green
            # Clean up PNGs
            Remove-Item $pngFiles
        } else {
            Write-Host "    × Failed to create ICO (keeping PNGs)" -ForegroundColor Red
        }
    } else {
        Write-Host "    ! ImageMagick not found - keeping PNGs" -ForegroundColor Yellow
        Write-Host "      Install ImageMagick or convert PNGs to ICO manually" -ForegroundColor Gray
    }
}

Write-Host "`nDone! Icon files created in resources\" -ForegroundColor Green

if (-not $magickPath) {
    Write-Host "`nTo convert PNGs to ICO files:" -ForegroundColor Cyan
    Write-Host "  Option 1: Install ImageMagick (https://imagemagick.org)" -ForegroundColor Gray
    Write-Host "  Option 2: Use online converter (convertio.co, icoconvert.com)" -ForegroundColor Gray
    Write-Host "  Option 3: Use GIMP - open PNGs, export as .ico with all sizes" -ForegroundColor Gray
}
