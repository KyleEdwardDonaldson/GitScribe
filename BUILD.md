# Build Instructions

## Issue with Git Bash

The Git Bash environment has a `link` utility that conflicts with Microsoft's `link.exe` linker needed for Rust builds.

**Solution**: Use PowerShell or Command Prompt instead.

## Building gitscribe-core

### Option 1: PowerShell (Recommended)

Open **PowerShell** and run:

```powershell
cd C:\R\GitScribe\gitscribe-core
cargo build
```

That's it! Cargo will automatically find the MSVC toolchain.

### Option 2: Command Prompt with VS Developer Tools

Open **"Developer PowerShell for VS 2022"** from Start Menu, then:

```cmd
cd C:\R\GitScribe\gitscribe-core
cargo build
```

### Option 3: Using the build script

We've created `build.bat` that sets up the environment:

```cmd
cd C:\R\GitScribe\gitscribe-core
build.bat
```

## Expected Output

First build will take 2-5 minutes as it downloads and compiles dependencies.

You should see:

```
   Compiling git2 v0.19.0
   Compiling rusqlite v0.32.1
   Compiling gitscribe-core v0.1.0 (C:\R\GitScribe\gitscribe-core)
    Finished dev [unoptimized + debuginfo] target(s) in 2m 34s
```

## Output Files

After successful build:

- **DLL**: `target\debug\gitscribe_core.dll`
- **Static lib**: `target\debug\gitscribe_core.lib`
- **C header**: Will be generated in Week 2

## Running Tests

```powershell
cargo test
```

## Troubleshooting

### "link.exe not found" error

Make sure Visual Studio 2022 with "Desktop development with C++" is installed.

### "cannot find link.exe"

Run from PowerShell or VS Developer PowerShell, not Git Bash.

### Build takes forever

First build is slow (downloads ~100 crates). Subsequent builds are fast.

---

**Next**: Once this builds successfully, we're done with Week 1, Day 3-4! 🎉
