# winget Package Manifest

Manifests for publishing GitScribe Status to the Windows Package Manager (winget).

## Prerequisites

1. **Create installer** - Need a proper `.exe` or `.msi` installer
2. **Host on GitHub Releases** - Installer must have stable URL
3. **Get SHA256 hash** - Required for manifest

## Creating the Manifest

### Option 1: Using WingetCreate (Recommended)

```powershell
# Install wingetcreate
winget install Microsoft.WingetCreate

# Generate manifest interactively
wingetcreate new https://github.com/KyleEdwardDonaldson/GitScribe/releases/download/status-v0.1.0/GitScribeStatus-Setup.exe

# Update existing manifest
wingetcreate update KyleDonaldson.GitScribe.Status -u https://github.com/KyleEdwardDonaldson/GitScribe/releases/download/status-v0.1.0/GitScribeStatus-Setup.exe -v 0.1.0
```

### Option 2: Manual Creation

See `manifests/` directory for YAML templates.

## Submitting to winget-pkgs

1. **Fork the repo:**
   ```bash
   gh repo fork microsoft/winget-pkgs
   ```

2. **Add your manifests:**
   ```bash
   cd winget-pkgs
   mkdir -p manifests/k/KyleDonaldson/GitScribe/Status/0.1.0/
   cp /path/to/your/manifests/* manifests/k/KyleDonaldson/GitScribe/Status/0.1.0/
   ```

3. **Validate:**
   ```powershell
   # Install winget validation tool
   winget install Microsoft.WingetCreate

   # Validate manifests
   winget validate manifests/k/KyleDonaldson/GitScribe/Status/0.1.0/
   ```

4. **Create PR:**
   ```bash
   git checkout -b add-gitscribe-status-0.1.0
   git add manifests/k/KyleDonaldson/GitScribe/Status/0.1.0/
   git commit -m "New package: KyleDonaldson.GitScribe.Status version 0.1.0"
   git push origin add-gitscribe-status-0.1.0
   gh pr create --title "New package: KyleDonaldson.GitScribe.Status version 0.1.0"
   ```

## Manifest Structure

winget requires 3 files:

1. **Installer manifest** (`KyleDonaldson.GitScribe.Status.installer.yaml`)
   - Installer type, architecture, download URL, SHA256

2. **Locale manifest** (`KyleDonaldson.GitScribe.Status.locale.en-US.yaml`)
   - Description, license, tags, etc.

3. **Version manifest** (`KyleDonaldson.GitScribe.Status.yaml`)
   - Links the above two together

## Getting SHA256 Hash

```powershell
# PowerShell
Get-FileHash GitScribeStatus-Setup.exe -Algorithm SHA256

# Or use certutil (built into Windows)
certutil -hashfile GitScribeStatus-Setup.exe SHA256
```

## Package ID Guidelines

**Format:** `Publisher.Application.Package`

For GitScribe Status: `KyleDonaldson.GitScribe.Status`

- **Publisher:** Your name or company
- **Application:** GitScribe
- **Package:** Status (to differentiate from Full version)

## Links

- **winget-pkgs repo:** https://github.com/microsoft/winget-pkgs
- **Manifest documentation:** https://github.com/microsoft/winget-pkgs/tree/master/doc
- **WingetCreate:** https://github.com/microsoft/winget-create
