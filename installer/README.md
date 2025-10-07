# GitScribe Status Installer

This directory contains the WiX Toolset installer project for **GitScribe Status**.

## Prerequisites

### Required Tools

1. **WiX Toolset v3.11 or later**
   ```powershell
   winget install WixToolset.WixToolset
   ```
   Or download from: https://wixtoolset.org/releases/

2. **Built binaries** (from gitscribe-shell and gitscribe-core)
   - `GitScribeShell.dll`
   - `gitscribe_core.dll`
   - Icon resources in `resources/icons/`

### Building the Binaries

Before building the installer, you must build the shell extension and core library:

```bash
# 1. Build core library (Rust)
cd gitscribe-core
cargo build --release

# 2. Build shell extension (C++)
cd ../gitscribe-shell
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release

# 3. Copy core DLL to shell extension output
copy ..\..\gitscribe-core\target\release\gitscribe_core.dll bin\Release\
```

## Building the Installer

### Using the Build Script (Recommended)

```cmd
cd installer
build.cmd Release
```

The installer will be created at: `installer\output\GitScribe-Status-Setup.msi`

### Manual Build

```cmd
cd installer

REM Compile
candle.exe -nologo ^
    -dBinDir="..\gitscribe-shell\build\bin\Release" ^
    -dIconsDir="..\gitscribe-shell\resources\icons" ^
    -arch x64 ^
    -out output\Product.wixobj ^
    Product.wxs

REM Link
light.exe -nologo ^
    -ext WixUIExtension ^
    -cultures:en-US ^
    -out output\GitScribe-Status-Setup.msi ^
    output\Product.wixobj
```

## Testing the Installer

### Install with Logging

```cmd
msiexec /i output\GitScribe-Status-Setup.msi /l*v install.log
```

Check `install.log` for any errors.

### Silent Install

```cmd
msiexec /i output\GitScribe-Status-Setup.msi /quiet /norestart
```

### Uninstall

```cmd
msiexec /x output\GitScribe-Status-Setup.msi
```

## Installer Features

### What It Does

1. ✅ **Checks prerequisites**:
   - Windows 10 1809+ or Windows 11
   - Administrator privileges
   - Visual C++ 2022 Redistributable (x64)

2. ✅ **Installs files**:
   - DLLs to `C:\Program Files\GitScribe\`
   - Icons to `C:\Program Files\GitScribe\icons\`
   - Documentation (README, INSTALL, TROUBLESHOOTING)

3. ✅ **Registers shell extension**:
   - Runs `regsvr32 /s GitScribeShell.dll`
   - Creates registry entries for overlay icons

4. ✅ **Restarts Explorer**:
   - Kills `explorer.exe`
   - Starts `explorer.exe`

5. ✅ **Creates uninstall entry**:
   - Appears in "Programs and Features"
   - Includes version, publisher, help links

### What It Doesn't Do (Yet)

- ❌ Bundle VC++ Redistributable (users must install separately)
- ❌ Create desktop shortcut (no app to launch yet)
- ❌ Auto-update mechanism (planned for GitScribe Full)

## Installer Size

Expected size: **~2 MB**

Breakdown:
- `GitScribeShell.dll`: ~419 KB
- `gitscribe_core.dll`: ~4 MB (compressed to ~1.5 MB in MSI)
- Icons: ~100 KB
- Documentation: ~50 KB

## Customization

### Change Icon Theme Default

Edit `Product.wxs` line ~215:

```xml
<RegistryValue Type="string" Name="IconTheme" Value="classic" />
```

Change `"classic"` to any theme: `minimalist`, `bold`, `neon`, etc.

### Change Install Location

Users can change install location during interactive install. Default is:

```
C:\Program Files\GitScribe
```

### Add More Files

Edit `Product.wxs` and add to `<ComponentGroup Id="ProductComponents">`:

```xml
<Component Id="NewFile" Guid="{NEW-GUID-HERE}" Win64="yes">
  <File Id="NewFile" Source="path\to\file.ext" />
</Component>
```

**Important**: Generate new GUIDs for each component:

```powershell
[guid]::NewGuid()
```

## Signing the Installer (For Release)

### Code Signing Certificate Required

```cmd
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com output\GitScribe-Status-Setup.msi
```

### Self-Signed for Testing

```powershell
# Create self-signed cert
$cert = New-SelfSignedCertificate -Subject "CN=GitScribe" -Type CodeSigning -CertStoreLocation Cert:\CurrentUser\My

# Export
Export-PfxCertificate -Cert $cert -FilePath .\test-cert.pfx -Password (ConvertTo-SecureString -String "test123" -Force -AsPlainText)

# Sign
signtool sign /f test-cert.pfx /p test123 output\GitScribe-Status-Setup.msi
```

## Troubleshooting

### Error: "WiX Toolset not found"

Install WiX Toolset and add to PATH:

```powershell
$env:Path += ";C:\Program Files (x86)\WiX Toolset v3.11\bin"
```

### Error: "GitScribeShell.dll not found"

Build the shell extension first (see Prerequisites).

### Error: "Access denied" during install

Run installer as Administrator or use elevated command prompt:

```cmd
msiexec /i output\GitScribe-Status-Setup.msi
```

### Installer builds but won't install

Check Windows Event Viewer:
- Application Log → Filter for "MsiInstaller"

Or review install log:

```cmd
notepad install.log
```

Search for "Return value 3" (errors).

### Explorer doesn't restart after install

Manually restart:

```cmd
taskkill /f /im explorer.exe
start explorer.exe
```

## Creating winget Manifest

After releasing v0.1.0, submit to winget:

1. Fork https://github.com/microsoft/winget-pkgs
2. Create manifest:
   ```
   manifests/g/GitScribe/Status/0.1.0/
   ├── GitScribe.Status.installer.yaml
   ├── GitScribe.Status.locale.en-US.yaml
   └── GitScribe.Status.yaml
   ```
3. Submit pull request

See: https://docs.microsoft.com/en-us/windows/package-manager/package/manifest

## Future Improvements

- [ ] Bundle with VC++ Redistributable using Burn (WiX Toolset v4)
- [ ] Create EXE wrapper instead of MSI
- [ ] Add custom UI branding
- [ ] Implement auto-update check
- [ ] Add telemetry opt-in (during install)
- [ ] Create portable version (no install)

## Resources

- **WiX Documentation**: https://wixtoolset.org/documentation/
- **Tutorial**: https://www.firegiant.com/wix/tutorial/
- **Schema Reference**: https://wixtoolset.org/documentation/manual/v3/xsd/

---

**Questions?** See main README or contact team@gitscribe.dev
