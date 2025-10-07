# GitScribe Release Process

Step-by-step guide for releasing GitScribe products.

## Pre-Release Checklist

- [ ] All tests pass
- [ ] Build succeeds locally
- [ ] Documentation is updated
- [ ] CHANGELOG.md is updated
- [ ] Version numbers are bumped

---

## Releasing GitScribe Status

### 1. Build the Installer

```powershell
# Build the Status version
cd gitscribe-shell
.\build-status.ps1

# Output will be in: build-status\bin\Release\
# - GitScribeShell.dll
# - gitscribe_core.dll
```

**TODO:** Create WiX installer script (see `installer/` directory)

For now, you can create a simple zip:
```powershell
cd build-status\bin\Release
Compress-Archive -Path GitScribeShell.dll,gitscribe_core.dll,..\..\resources -DestinationPath GitScribeStatus-v0.1.0-win64.zip
```

### 2. Create GitHub Release

**Via GitHub Web UI:**

1. Go to https://github.com/KyleEdwardDonaldson/GitScribe/releases
2. Click **"Draft a new release"**
3. Fill in the form:

**Tag version:** `status-v0.1.0`
- Use format: `status-v{MAJOR}.{MINOR}.{PATCH}`
- Target: `main` branch

**Release title:** `GitScribe Status v0.1.0 - Initial Beta`

**Description:**
```markdown
# GitScribe Status v0.1.0 🎉

**Initial beta release** of GitScribe Status - the lightweight Git shell extension for Windows.

## What's Included

✅ **Overlay icons** for all Git file states
✅ **Context menu** showing "GitScribe | Clean/Modified/Merging/etc."
✅ **Click to copy** file path functionality
✅ **Fast cached status** updates (<50ms)
✅ **Standalone** - no app dependency (~2MB)

## Installation

### Manual Installation (Current)

1. Download `GitScribeStatus-v0.1.0-win64.zip`
2. Extract to `C:\Program Files\GitScribe\`
3. Run as Administrator:
   ```cmd
   regsvr32 "C:\Program Files\GitScribe\GitScribeShell.dll"
   ```
4. Restart Windows Explorer:
   ```cmd
   taskkill /f /im explorer.exe
   start explorer.exe
   ```

### Coming Soon
- Windows Installer (.msi or .exe)
- winget: `winget install GitScribe.Status`
- Chocolatey: `choco install gitscribe-status`

## What's Different from Full Version?

| Feature | Status (This Release) | Full (Coming Soon) |
|---------|----------------------|-------------------|
| Overlay icons | ✅ | ✅ |
| Context menu | ✅ Status display only | ✅ Full operations |
| Click action | Copy path | Open submenu |
| Size | ~2MB | ~50MB+ |
| Requires app | ❌ | ✅ |

## Known Limitations

- No operations menu (commit, push, pull) - status display only
- No property sheet tab
- Manual installation required (installer coming soon)

## Requirements

- Windows 10/11 (x64)
- Git installed (any version)
- Administrator access for registration

## Uninstallation

```cmd
regsvr32 /u "C:\Program Files\GitScribe\GitScribeShell.dll"
taskkill /f /im explorer.exe
start explorer.exe
```

## Changelog

See [CHANGELOG.md](https://github.com/KyleEdwardDonaldson/GitScribe/blob/main/CHANGELOG.md) or [Web Changelog](https://gitscri.be/changelog)

## Next Steps

- 📦 Create proper installer (.msi)
- 🏪 Publish to winget and Chocolatey
- 🐛 Bug fixes and stability improvements
- ➡️ Begin work on GitScribe Full

## Links

- **Website:** https://gitscri.be
- **Changelog:** https://gitscri.be/changelog
- **Documentation:** https://github.com/KyleEdwardDonaldson/GitScribe/blob/main/BUILD_STATUS.md
- **Report Issues:** https://github.com/KyleEdwardDonaldson/GitScribe/issues

---

**Built with ❤️ by Kyle Donaldson**
```

4. **Attach files:**
   - Upload `GitScribeStatus-v0.1.0-win64.zip`

5. **Pre-release:** ✅ Check "Set as a pre-release" (since it's v0.1.0 beta)

6. Click **"Publish release"**

**Via GitHub CLI:**

```bash
# Create release
gh release create status-v0.1.0 \
  --title "GitScribe Status v0.1.0 - Initial Beta" \
  --notes-file release-notes.md \
  --prerelease \
  GitScribeStatus-v0.1.0-win64.zip

# Where release-notes.md contains the markdown above
```

### 3. Update Documentation Links

After release is published, update any broken links:

- `README.md` - Update download links
- Landing site - Verify changelog links work
- CHANGELOG.md - Confirm release tag exists

---

## Publishing to Distribution Platforms

### Windows Package Manager (winget)

**Most important for Windows users!**

1. **Prerequisites:**
   - Installer must be hosted with stable URL (GitHub releases work)
   - SHA256 hash of installer
   - Manifest file

2. **Create manifest:**
   ```powershell
   # Install winget package creator
   winget install Microsoft.WingetCreate

   # Create manifest (interactive)
   wingetcreate new https://github.com/KyleEdwardDonaldson/GitScribe/releases/download/status-v0.1.0/GitScribeStatus-Setup.exe
   ```

3. **Submit to winget-pkgs:**
   - Fork https://github.com/microsoft/winget-pkgs
   - Add manifest to `manifests/k/KyleDonaldson/GitScribe/Status/0.1.0/`
   - Create pull request

**See:** `installer/winget/` directory for manifest template

### Chocolatey

**Popular Windows package manager**

1. **Create nuspec:**
   ```xml
   <!-- gitscribe-status.nuspec -->
   <?xml version="1.0" encoding="utf-8"?>
   <package xmlns="http://schemas.microsoft.com/packaging/2015/06/nuspec.xsd">
     <metadata>
       <id>gitscribe-status</id>
       <version>0.1.0</version>
       <title>GitScribe Status</title>
       <authors>Kyle Donaldson</authors>
       <projectUrl>https://gitscri.be</projectUrl>
       <license type="expression">MPL-2.0</license>
       <requireLicenseAcceptance>false</requireLicenseAcceptance>
       <description>Lightweight Git shell extension for Windows with overlay icons and status display.</description>
       <tags>git windows shell-extension overlay-icons</tags>
     </metadata>
     <files>
       <file src="tools\**" target="tools" />
     </files>
   </package>
   ```

2. **Create install script:**
   ```powershell
   # tools\chocolateyinstall.ps1
   $ErrorActionPreference = 'Stop'
   $packageName = 'gitscribe-status'
   $url64 = 'https://github.com/KyleEdwardDonaldson/GitScribe/releases/download/status-v0.1.0/GitScribeStatus-Setup.exe'
   $checksum64 = 'YOUR_SHA256_HASH_HERE'

   $packageArgs = @{
     packageName   = $packageName
     fileType      = 'EXE'
     url64bit      = $url64
     checksum64    = $checksum64
     checksumType64= 'sha256'
     silentArgs    = '/VERYSILENT /SUPPRESSMSGBOXES /NORESTART'
   }

   Install-ChocolateyPackage @packageArgs
   ```

3. **Publish:**
   ```powershell
   choco pack
   choco push gitscribe-status.0.1.0.nupkg --source https://push.chocolatey.org/
   ```

**See:** `installer/chocolatey/` directory

### Scoop

**Another Windows package manager**

1. **Create manifest:**
   ```json
   {
     "version": "0.1.0",
     "description": "Lightweight Git shell extension for Windows",
     "homepage": "https://gitscri.be",
     "license": "MPL-2.0",
     "url": "https://github.com/KyleEdwardDonaldson/GitScribe/releases/download/status-v0.1.0/GitScribeStatus-v0.1.0-win64.zip",
     "hash": "sha256:YOUR_HASH_HERE",
     "extract_dir": "GitScribeStatus-v0.1.0",
     "installer": {
       "script": [
         "regsvr32 /s \"$dir\\GitScribeShell.dll\"",
         "Write-Host 'Please restart Windows Explorer' -ForegroundColor Yellow"
       ]
     },
     "uninstaller": {
       "script": "regsvr32 /s /u \"$dir\\GitScribeShell.dll\""
     }
   }
   ```

2. **Submit:**
   - Fork https://github.com/ScoopInstaller/Main
   - Add manifest to `bucket/gitscribe-status.json`
   - Create pull request

---

## Distribution Platform Summary

| Platform | Priority | Users | Command |
|----------|----------|-------|---------|
| **GitHub Releases** | 🔥 Critical | All users | Manual download |
| **winget** | 🔥 High | Windows 10/11+ (built-in) | `winget install GitScribe.Status` |
| **Chocolatey** | ⭐ Medium | Windows power users | `choco install gitscribe-status` |
| **Scoop** | ⭐ Low | Windows developers | `scoop install gitscribe-status` |
| **Microsoft Store** | 💰 Optional | Mainstream users | Costs $19/year registration |

### Recommended Order

**Phase 1 (Now - Manual Distribution):**
1. ✅ **GitHub Releases** - Host the installer zip
   - Free
   - Users download manually
   - Works immediately

**Phase 2 (After proper .msi/.exe installer):**
2. 🎯 **winget** - Official Windows package manager
   - Built into Windows 10/11
   - Maintained by Microsoft
   - Free and most legitimate
   - **Priority #1 for package managers**

3. 📦 **Chocolatey** - Popular third-party package manager
   - Large existing user base
   - Easy to maintain
   - Good for power users

**Phase 3 (Lower Priority):**
4. **Scoop** - Developer-focused package manager
   - Smaller audience
   - Easy to add after Chocolatey

5. **Microsoft Store** - Only if going mainstream
   - Costs $19/year developer registration
   - Good for non-technical users
   - Automatic updates built-in
   - Not worth it until you have traction

### Why Not crates.io?

**gitscribe-core is not published to crates.io** because:
- It's GitScribe-specific, not a general-purpose library
- Developers needing Git operations would use `git2` directly
- No real audience for a GitScribe-specific Rust crate
- Maintenance burden without benefit

**Focus on Windows package managers instead** - that's where your users are.

---

## Post-Release Checklist

After creating GitHub release:

- [ ] Verify download links work
- [ ] Test installation on clean VM
- [ ] Update website changelog
- [ ] Tweet/post announcement (if applicable)
- [ ] Monitor GitHub issues for bug reports
- [ ] Plan next release based on feedback

---

## Automated Releases (Future)

Consider GitHub Actions for automated releases:

```yaml
# .github/workflows/release-status.yml
name: Release GitScribe Status

on:
  push:
    tags:
      - 'status-v*'

jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build Rust core
        run: |
          cd gitscribe-core
          cargo build --release
      - name: Build Shell Extension
        run: |
          cd gitscribe-shell
          .\build-status.ps1
      - name: Create Release
        uses: softprops/action-gh-release@v1
        with:
          files: |
            gitscribe-shell/build-status/bin/Release/GitScribeStatus-*.zip
          prerelease: ${{ contains(github.ref, 'alpha') || contains(github.ref, 'beta') }}
```

---

**Questions?** Open an issue or email kyle@gitscri.be
