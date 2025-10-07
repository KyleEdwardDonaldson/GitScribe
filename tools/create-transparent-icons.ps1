# Create transparent overlay icons for GitScribe
Add-Type -AssemblyName System.Drawing

$iconDir = "gitscribe-shell\resources\icon-packs\default"

# Icon configurations: name, color, symbol
$icons = @(
    @{Name='modified'; Color='255,165,0'; Symbol='M'},
    @{Name='added'; Color='0,200,0'; Symbol='+'},
    @{Name='clean'; Color='0,150,255'; Symbol='V'},
    @{Name='untracked'; Color='128,128,128'; Symbol='?'},
    @{Name='conflicted'; Color='255,0,0'; Symbol='!'},
    @{Name='ignored'; Color='180,180,180'; Symbol='X'}
)

foreach ($icon in $icons) {
    $name = $icon.Name
    $rgb = $icon.Color.Split(",")
    $color = [System.Drawing.Color]::FromArgb(255, [int]$rgb[0], [int]$rgb[1], [int]$rgb[2])
    $symbol = $icon.Symbol

    # Create 16x16 bitmap with transparency
    $bitmap = New-Object System.Drawing.Bitmap(16, 16)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.Clear([System.Drawing.Color]::Transparent)

    # Draw a small colored indicator in bottom-right corner
    $brush = New-Object System.Drawing.SolidBrush($color)
    $graphics.FillEllipse($brush, 8, 8, 7, 7)

    # Draw symbol
    $font = New-Object System.Drawing.Font("Arial", 6, [System.Drawing.FontStyle]::Bold)
    $textBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
    $graphics.DrawString($symbol, $font, $textBrush, 9, 8)

    # Save as PNG first (ICO support is limited in .NET)
    $pngPath = "$iconDir\$name.png"
    $bitmap.Save($pngPath, [System.Drawing.Imaging.ImageFormat]::Png)

    # Convert PNG to ICO using built-in .NET (simple single-size ICO)
    $icon16 = [System.Drawing.Icon]::FromHandle($bitmap.GetHicon())
    $stream = [System.IO.File]::Create("$iconDir\$name.ico")
    $icon16.Save($stream)
    $stream.Close()

    $graphics.Dispose()
    $bitmap.Dispose()
    $brush.Dispose()
    $textBrush.Dispose()
    $font.Dispose()

    Write-Host "Created $name.ico" -ForegroundColor Green
}

Write-Host "`nAll icons created successfully!" -ForegroundColor Cyan
