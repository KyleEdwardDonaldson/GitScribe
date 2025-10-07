Add-Type -AssemblyName System.Drawing

$pngPath = "C:\r\GitScribe\resources\gitscribe-menu-icon.png"
$icoPath = "C:\r\GitScribe\resources\gitscribe-menu-icon.ico"

# Load the PNG
$img = [System.Drawing.Image]::FromFile($pngPath)

# Create 16x16 bitmap
$bmp = New-Object System.Drawing.Bitmap(16, 16)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
$g.DrawImage($img, 0, 0, 16, 16)
$g.Dispose()

# Save as ICO
$stream = [System.IO.File]::Create($icoPath)
$bmp.Save($stream, [System.Drawing.Imaging.ImageFormat]::Icon)
$stream.Close()

$bmp.Dispose()
$img.Dispose()

Write-Host "Icon created at $icoPath" -ForegroundColor Green
