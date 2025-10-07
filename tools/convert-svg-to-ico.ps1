# Convert extracted SVG icon packs to ICO files for Windows shell overlays
# Requires: ImageMagick with librsvg OR Inkscape

param(
    [string]$IconPacksDir = "C:\R\GitScribe\gitscribe-shell\resources\icon-packs",
    [int[]]$Sizes = @(16, 32, 48, 128)
)

Write-Host "GitScribe SVG → ICO Converter" -ForegroundColor Cyan
Write-Host "==============================`n" -ForegroundColor Cyan

# Check for conversion tools
$magick = Get-Command "magick" -ErrorAction SilentlyContinue
$inkscape = Get-Command "inkscape" -ErrorAction SilentlyContinue

if (-not $magick -and -not $inkscape) {
    Write-Host "ERROR: Neither ImageMagick nor Inkscape found!" -ForegroundColor Red
    Write-Host "`nPlease install one of:" -ForegroundColor Yellow
    Write-Host "  • ImageMagick: https://imagemagick.org/script/download.php" -ForegroundColor Gray
    Write-Host "  • Inkscape: https://inkscape.org/release/" -ForegroundColor Gray
    exit 1
}

$converter = if ($inkscape) { "inkscape" } else { "magick" }
Write-Host "Using converter: $converter`n" -ForegroundColor Green

# Find all icon pack directories
$packDirs = Get-ChildItem -Path $IconPacksDir -Directory

foreach ($packDir in $packDirs) {
    Write-Host "Converting: $($packDir.Name)" -ForegroundColor Yellow

    # Find all SVG files
    $svgFiles = Get-ChildItem -Path $packDir.FullName -Filter "*.svg"

    foreach ($svgFile in $svgFiles) {
        $baseName = $svgFile.BaseName
        Write-Host "  Processing $baseName.svg..." -ForegroundColor Gray

        # Generate PNGs at different sizes
        $pngFiles = @()
        foreach ($size in $Sizes) {
            $pngPath = Join-Path $packDir.FullName "$baseName-$size.png"

            if ($converter -eq "inkscape") {
                # Use Inkscape
                & inkscape --export-type=png `
                    --export-filename="$pngPath" `
                    --export-width=$size `
                    --export-height=$size `
                    "$($svgFile.FullName)" 2>$null
            } else {
                # Use ImageMagick
                & magick convert -background none `
                    -resize "${size}x${size}" `
                    "$($svgFile.FullName)" `
                    "$pngPath" 2>$null
            }

            if (Test-Path $pngPath) {
                $pngFiles += $pngPath
            } else {
                Write-Host "    × Failed to create ${size}x${size} PNG" -ForegroundColor Red
            }
        }

        if ($pngFiles.Count -eq $Sizes.Count) {
            # Convert PNGs to single ICO file
            $icoPath = Join-Path $packDir.FullName "$baseName.ico"

            if ($converter -eq "magick") {
                & magick convert $pngFiles "$icoPath" 2>$null
            } else {
                # ImageMagick is better for ICO, try to use it even if using Inkscape for SVG
                $magickForIco = Get-Command "magick" -ErrorAction SilentlyContinue
                if ($magickForIco) {
                    & magick convert $pngFiles "$icoPath" 2>$null
                } else {
                    Write-Host "    ! Cannot create ICO without ImageMagick" -ForegroundColor Yellow
                    Write-Host "      Use an online converter or install ImageMagick" -ForegroundColor Gray
                    continue
                }
            }

            if (Test-Path $icoPath) {
                Write-Host "    ✓ Created $baseName.ico" -ForegroundColor Green

                # Clean up PNG files
                foreach ($png in $pngFiles) {
                    Remove-Item $png -Force
                }
            } else {
                Write-Host "    × Failed to create ICO file" -ForegroundColor Red
            }
        } else {
            Write-Host "    × Incomplete PNG set, skipping ICO creation" -ForegroundColor Red
        }
    }
}

Write-Host "`n✓ Conversion complete!" -ForegroundColor Green
Write-Host "`nIcon packs are ready in: $IconPacksDir" -ForegroundColor Cyan
