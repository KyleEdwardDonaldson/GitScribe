# Check GitScribe registration status

Write-Host "=== Checking Overlay Handlers ===" -ForegroundColor Cyan
Get-ChildItem "Registry::HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers" |
    Where-Object { $_.PSChildName -like "*GitScribe*" } |
    ForEach-Object {
        $name = $_.PSChildName
        $clsid = $_.GetValue('')
        Write-Host "$name = $clsid" -ForegroundColor Green
    }

Write-Host "`n=== Checking Context Menu Handlers ===" -ForegroundColor Cyan
$paths = @(
    "HKEY_CLASSES_ROOT\*\shellex\ContextMenuHandlers",
    "HKEY_CLASSES_ROOT\Directory\shellex\ContextMenuHandlers",
    "HKEY_CLASSES_ROOT\Directory\Background\shellex\ContextMenuHandlers"
)

foreach ($path in $paths) {
    try {
        $items = Get-ChildItem "Registry::$path" -ErrorAction Stop |
            Where-Object { $_.PSChildName -like "*GitScribe*" }
        foreach ($item in $items) {
            $name = $item.PSChildName
            $clsid = $item.GetValue('')
            Write-Host "$path\$name = $clsid" -ForegroundColor Green
        }
    } catch {
        # Path might not exist
    }
}

Write-Host "`n=== Checking Property Sheet Handlers ===" -ForegroundColor Cyan
$paths = @(
    "HKEY_CLASSES_ROOT\*\shellex\PropertySheetHandlers",
    "HKEY_CLASSES_ROOT\Directory\shellex\PropertySheetHandlers"
)

foreach ($path in $paths) {
    try {
        $items = Get-ChildItem "Registry::$path" -ErrorAction Stop |
            Where-Object { $_.PSChildName -like "*GitScribe*" }
        foreach ($item in $items) {
            $name = $item.PSChildName
            $clsid = $item.GetValue('')
            Write-Host "$path\$name = $clsid" -ForegroundColor Green
        }
    } catch {
        # Path might not exist
    }
}

Write-Host "`n=== Checking CLSIDs ===" -ForegroundColor Cyan
# Try to find GitScribe CLSIDs
$clsids = @(
    "{E1A2F5D0-9B3C-4A1E-8F2D-3C4B5A6E7F89}",  # ModifiedOverlay
    "{E1A2F5D0-9B3C-4A1E-8F2D-3C4B5A6E7F8A}",  # CleanOverlay
    "{E1A2F5D0-9B3C-4A1E-8F2D-3C4B5A6E7F8B}",  # AddedOverlay
    "{E1A2F5D0-9B3C-4A1E-8F2D-3C4B5A6E7F8C}",  # UntrackedOverlay
    "{E1A2F5D0-9B3C-4A1E-8F2D-3C4B5A6E7F8D}",  # ConflictedOverlay
    "{E1A2F5D0-9B3C-4A1E-8F2D-3C4B5A6E7F8E}",  # IgnoredOverlay
    "{E1A2F5D0-9B3C-4A1E-8F2D-3C4B5A6E7F90}",  # ContextMenu
    "{E1A2F5D0-9B3C-4A1E-8F2D-3C4B5A6E7F91}"   # PropertySheet
)

foreach ($clsid in $clsids) {
    try {
        $reg = Get-Item "Registry::HKEY_CLASSES_ROOT\CLSID\$clsid" -ErrorAction Stop
        $desc = $reg.GetValue('')
        $dll = Get-ItemProperty "Registry::HKEY_CLASSES_ROOT\CLSID\$clsid\InprocServer32" -ErrorAction Stop
        Write-Host "$clsid ($desc)" -ForegroundColor Yellow
        Write-Host "  DLL: $($dll.'(default)')" -ForegroundColor Gray
    } catch {
        Write-Host "$clsid - NOT REGISTERED" -ForegroundColor Red
    }
}
