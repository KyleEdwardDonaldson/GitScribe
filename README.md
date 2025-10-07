# GitScribe

**Modern Windows Git Client** - Fast, reliable Windows Explorer integration without the crashes, freezes, and frustration.

![License](https://img.shields.io/badge/license-Open%20Core-blue)
![Platform](https://img.shields.io/badge/platform-Windows-blue)
![Status](https://img.shields.io/badge/status-In%20Development-yellow)

## 🎯 Project Vision

GitScribe combines the contextual convenience of TortoiseGit with modern performance and reliability. Built with Rust for safety and speed, it provides seamless Windows Explorer integration that actually works.

## 📦 Components

This is a monorepo containing all GitScribe components:

### Open Source (MPL-2.0)
- **[gitscribe-core](./gitscribe-core)** - Rust core library for Git operations
- **[gitscribe-shell](./gitscribe-shell)** - Windows shell extension (overlay icons + context menus)

### Proprietary (Closed Source)
- **gitscribe-app** - Electron application (commit dialogs, history, etc.) - *Not included in this repository*
- **gitscribe-landing** - Marketing website - *Not included in this repository*

### Open Source (MIT)
- **[gitscribe-plugins](./gitscribe-plugins)** - Plugin SDK for extensions

## 🚀 Quick Start

### GitScribe Status (Free, Overlay Icons Only)

Download the lightweight version for just visual Git status indicators:

```powershell
# Coming soon - 2MB installer
winget install GitScribe.Status
```

**Features:**
- Status overlay icons (modified, added, deleted, etc.)
- Cached status updates < 50ms
- No dependencies, minimal resources

### GitScribe Full (Coming Soon)

Full-featured Git client with context menus, commit dialogs, and more.

## 🏗️ Architecture

### Open Core Model

GitScribe Status (the shell extension) is **open source** under MPL-2.0:
- View and modify the code
- Use in proprietary software
- Contribute improvements
- Build your own tools on top

GitScribe Full (the Electron app) is **proprietary but source-available**:
- Source code visible for security audits
- Free for personal use
- Enterprise licensing available

### Technology Stack

- **Core**: Rust + libgit2 for performance and safety
- **Shell Extension**: C++ COM components for Windows integration
- **Application**: Electron + React + TypeScript
- **Plugin System**: QuickJS/WASM sandbox for safe extensions

## 💻 Development

### Prerequisites

- **Windows 10/11** (development OS)
- **Rust 1.70+** (for core library)
- **Visual Studio 2022** (for shell extension)
- **Node.js 20+** (for app and tooling)

### Building

```bash
# Clone the repository
git clone https://github.com/KyleEdwardDonaldson/GitScribe.git
cd gitscribe

# Build core library
cd gitscribe-core
cargo build --release

# Build shell extension
cd ../gitscribe-shell
msbuild gitscribe-shell.sln /p:Configuration=Release /p:Platform=x64

# Build application (when ready)
cd ../gitscribe-app
npm install
npm run build
```

See individual component READMEs for detailed build instructions.

## 🤝 Contributing

We welcome contributions, especially to the open source components!

### Open Source Components (Contributions Welcome)
- **gitscribe-core**: Performance improvements, Git operation coverage, bug fixes
- **gitscribe-shell**: Windows integration enhancements, stability fixes
- **gitscribe-plugins**: Plugin SDK improvements, example plugins

### Proprietary Components (Bug Reports Welcome)
- **gitscribe-app**: We accept bug reports and feature requests, but PRs are reviewed on a case-by-case basis

### How to Contribute

1. Open an issue to discuss major changes
2. Fork the repository
3. Create a feature branch
4. Make your changes with tests
5. Submit a pull request

See [CONTRIBUTING.md](./CONTRIBUTING.md) for detailed guidelines.

## 📄 License

GitScribe uses an **Open Core** licensing model:

| Component | License | Usage |
|-----------|---------|-------|
| gitscribe-core | MPL-2.0 | ✅ Free & Open Source |
| gitscribe-shell | MPL-2.0 | ✅ Free & Open Source |
| gitscribe-plugins | MIT | ✅ Free & Open Source |
| gitscribe-app | Proprietary | 🔒 Closed Source (Not Included) |
| gitscribe-landing | Proprietary | 🔒 Closed Source (Not Included) |

### What This Means

**Open Source (MPL-2.0/MIT):**
- ✅ Use in any project (commercial or personal)
- ✅ Modify and distribute
- ✅ Build derivative works
- ℹ️ Modifications to MPL-2.0 code must be shared

**Proprietary (Closed Source):**
- 🔒 Source code not publicly available
- 📦 Available as compiled binary only
- 💼 Enterprise source licensing available on request

## 🙏 Attribution

If you use GitScribe's open source components in your project, please include:

```
Built with GitScribe (https://github.com/KyleEdwardDonaldson/GitScribe)
Copyright © 2025 GitScribe
```

This helps others discover the project. Thank you!

## 🗺️ Roadmap

### ✅ Phase 0: Foundation (Current)
- [x] Repository structure
- [x] Core library basics
- [x] Shell extension prototype
- [x] Landing page

### 🏗️ Phase 1: MVP (In Progress)
- [ ] Complete shell integration
- [ ] Basic Git operations
- [ ] Commit/push/pull dialogs
- [ ] Status caching
- [ ] Beta release

### 📋 Phase 2: Enhancement
- [ ] Advanced Git operations (rebase, cherry-pick)
- [ ] Conflict resolution UI
- [ ] Multi-repo management
- [ ] Plugin system
- [ ] Public 1.0 release

### 🚀 Phase 3: Enterprise
- [ ] SSO/SAML authentication
- [ ] Audit logging
- [ ] Team collaboration features
- [ ] Enterprise pilot program

See [ROADMAP.md](./ROADMAP.md) for detailed plans.

## 📞 Contact & Support

- **Website**: [gitscribe.dev](https://gitscribe.dev) (coming soon)
- **Issues**: [GitHub Issues](https://github.com/KyleEdwardDonaldson/GitScribe/issues)
- **Email**: team@gitscribe.dev
- **Enterprise**: enterprise@gitscribe.dev

## ⭐ Show Your Support

If you find GitScribe useful:
- ⭐ Star this repository
- 🐦 Share on social media
- 💬 Tell your developer friends
- 🤝 Contribute code or documentation

## 📊 Project Stats

![GitHub stars](https://img.shields.io/github/stars/KyleEdwardDonaldson/GitScribe?style=social)
![GitHub forks](https://img.shields.io/github/forks/KyleEdwardDonaldson/GitScribe?style=social)
![GitHub issues](https://img.shields.io/github/issues/KyleEdwardDonaldson/GitScribe)
![GitHub pull requests](https://img.shields.io/github/issues-pr/KyleEdwardDonaldson/GitScribe)

---

**Built with ❤️ for the Windows developer community**
