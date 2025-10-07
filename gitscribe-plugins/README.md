# GitScribe Plugins & Marketplace

**Plugin SDK, marketplace infrastructure, and icon pack distribution for GitScribe**

## Overview

This repository contains everything needed to build, distribute, and consume GitScribe plugins and icon packs:

- **Plugin SDK** - Build extensions for GitScribe Full
- **Marketplace** - Web-based discovery and distribution platform
- **Icon Pack System** - Downloadable themes for GitScribe Status overlays
- **Client Libraries** - Download and install marketplace content

## Features

### Plugin SDK
- **JavaScript/TypeScript Plugins**: Write plugins in familiar languages
- **WASM Support**: High-performance plugins in Rust, Go, etc.
- **Safe Execution**: Sandboxed runtime prevents malicious code
- **Rich API**: Access to Git operations, UI components, and more
- **Hot Reload**: Develop plugins with instant feedback

### Marketplace
- **Web Integration**: Built into landing page (gitscribe.dev/marketplace)
- **Search & Discovery**: Full-text search, filters, categories, ratings
- **Security**: Automated malware scanning, manual review, permission system
- **Analytics**: Download tracking, popularity metrics
- **Reviews**: User ratings and comments

### Icon Pack Distribution
- **Minimal Installer**: Ship GitScribe Status with 1-2 default packs (~20 KB)
- **On-Demand Downloads**: Users download additional packs as needed (5-20 KB each)
- **Community Packs**: Easy submission process for designers
- **10+ Themes**: Neon, minimal, pixel art, gradients, and more

## Quick Start

### Create a Plugin

```bash
npm create gitscribe-plugin@latest my-plugin
cd my-plugin
npm install
npm run dev
```

### Plugin Structure

```
my-plugin/
├── package.json        # Plugin manifest
├── src/
│   ├── index.ts       # Entry point
│   └── ...
├── README.md
└── LICENSE.md
```

### Minimal Example

```typescript
// src/index.ts
import { Plugin, GitAPI, UI } from 'gitscribe-plugin-api';

export default class MyPlugin extends Plugin {
  name = 'My Plugin';
  version = '1.0.0';

  async onActivate() {
    // Register a context menu item
    this.contextMenu.register({
      id: 'my-action',
      title: 'Do Something Cool',
      onClick: async (files) => {
        await this.doSomething(files);
      }
    });
  }

  async doSomething(files: string[]) {
    UI.showNotification('Processing ' + files.length + ' files...');
    // Your logic here
  }
}
```

## API Reference

### Plugin Lifecycle

```typescript
class Plugin {
  // Called when plugin is loaded
  async onActivate(): Promise<void>;

  // Called when plugin is unloaded
  async onDeactivate(): Promise<void>;

  // Called when repository changes
  async onRepositoryChanged(path: string): Promise<void>;
}
```

### Git API

```typescript
interface GitAPI {
  // Repository operations
  getStatus(repoPath: string): Promise<FileStatus[]>;
  commit(repoPath: string, message: string): Promise<void>;
  push(repoPath: string, remote: string, branch: string): Promise<void>;
  pull(repoPath: string): Promise<void>;

  // Branch operations
  getBranches(repoPath: string): Promise<Branch[]>;
  createBranch(repoPath: string, name: string): Promise<void>;
  switchBranch(repoPath: string, name: string): Promise<void>;

  // History
  getLog(repoPath: string, options?: LogOptions): Promise<Commit[]>;
  getDiff(repoPath: string, commit1?: string, commit2?: string): Promise<string>;
}
```

### UI API

```typescript
interface UI {
  // Notifications
  showNotification(message: string, type?: 'info' | 'success' | 'warning' | 'error'): void;

  // Dialogs
  showDialog(options: DialogOptions): Promise<DialogResult>;
  showConfirm(message: string): Promise<boolean>;
  showInput(prompt: string, defaultValue?: string): Promise<string | null>;

  // Custom panels
  createPanel(id: string, title: string, content: React.Component): Panel;
}
```

### Context Menu API

```typescript
interface ContextMenuAPI {
  register(item: ContextMenuItem): void;
  unregister(id: string): void;
}

interface ContextMenuItem {
  id: string;
  title: string;
  icon?: string;
  condition?: (files: string[]) => boolean;
  onClick: (files: string[]) => Promise<void>;
}
```

### Settings API

```typescript
interface Settings {
  get<T>(key: string, defaultValue?: T): T;
  set(key: string, value: any): Promise<void>;
  onChanged(key: string, callback: (value: any) => void): void;
}
```

## Example Plugins

### Jira Integration

```typescript
export default class JiraPlugin extends Plugin {
  async onActivate() {
    this.contextMenu.register({
      id: 'jira-link',
      title: 'Link to Jira Issue',
      onClick: async () => {
        const issueKey = await UI.showInput('Enter Jira issue key:');
        if (issueKey) {
          const branch = await GitAPI.createBranch(
            this.currentRepo,
            `feature/${issueKey}`
          );
          UI.showNotification(`Branch created: ${branch.name}`);
        }
      }
    });
  }
}
```

### Auto-Format

```typescript
export default class AutoFormatPlugin extends Plugin {
  async onActivate() {
    this.hooks.beforeCommit(async (files) => {
      for (const file of files) {
        if (file.endsWith('.ts') || file.endsWith('.js')) {
          await this.formatFile(file);
        }
      }
    });
  }

  async formatFile(path: string) {
    // Run prettier or similar
  }
}
```

### Conventional Commits Helper

```typescript
export default class ConventionalCommitsPlugin extends Plugin {
  async onActivate() {
    this.hooks.beforeCommit(async (message) => {
      if (!this.isConventional(message)) {
        const result = await UI.showDialog({
          title: 'Non-conventional commit',
          message: 'Would you like help formatting this commit?',
          buttons: ['Format', 'Skip', 'Cancel']
        });

        if (result === 'Format') {
          return await this.formatMessage(message);
        }
      }
      return message;
    });
  }

  isConventional(message: string): boolean {
    return /^(feat|fix|docs|style|refactor|test|chore)(\(.+\))?: .+/.test(message);
  }
}
```

## Publishing

### To npm

```bash
npm run build
npm publish
```

### To GitScribe Plugin Registry

```bash
gitscribe plugin publish
```

## Development

### Hot Reload

```bash
npm run dev
```

Plugin will reload automatically when you save changes.

### Testing

```bash
npm test              # Unit tests
npm run test:e2e      # E2E tests in real GitScribe
```

### Debugging

```typescript
import { Logger } from 'gitscribe-plugin-api';

const log = Logger.create('my-plugin');
log.debug('Debug message');
log.info('Info message');
log.warn('Warning message');
log.error('Error message');
```

Logs appear in GitScribe's developer console.

## Security

Plugins run in a sandboxed environment:
- No access to file system outside repository
- No network access (unless explicitly allowed)
- No access to Node.js APIs
- Limited CPU and memory

To request additional permissions:

```json
// package.json
{
  "gitscribe": {
    "permissions": [
      "network",
      "filesystem:read",
      "filesystem:write"
    ]
  }
}
```

Users will be prompted to approve these permissions.

## Distribution

Plugins can be distributed via:
1. **npm**: Standard JavaScript package
2. **GitScribe Registry**: Official plugin marketplace
3. **GitHub**: Direct installation from repo
4. **File**: Manual `.gsplug` file installation

## TypeScript Support

Full TypeScript support out of the box:

```bash
npm install --save-dev @gitscribe/plugin-types
```

```typescript
import type { Plugin, GitAPI } from '@gitscribe/plugin-types';
```

## WASM Plugins

Write high-performance plugins in Rust:

```rust
use gitscribe_plugin_sdk::prelude::*;

#[plugin]
pub struct MyPlugin;

impl Plugin for MyPlugin {
    fn on_activate(&mut self) {
        // Your code
    }
}
```

Build to WASM:

```bash
cargo build --target wasm32-unknown-unknown
```

## Examples Repository

See https://github.com/gitscribe/plugin-examples for more examples:
- Azure DevOps integration
- GitHub Actions trigger
- Code review helper
- License checker
- And more...

## Marketplace

See **[MARKETPLACE.md](./MARKETPLACE.md)** for complete marketplace specification including:
- Architecture and data models
- User flows (browse, download, install)
- Security and moderation
- Submission process
- Analytics and metrics

See **[INTEGRATION.md](./INTEGRATION.md)** for landing page integration guide.

### Popular Plugins (Coming Soon)
1. **Conventional Commits** - Auto-format commit messages ([example](./examples/conventional-commits))
2. **AI Commit Message Generator** (Pro) - Generate messages from diffs
3. **Pre-Commit Security Scanner** - Detect secrets before committing
4. **Jira Integration** - Link commits to Jira issues
5. **GitHub Actions Status** - Show CI/CD status in commit view

### Icon Pack Gallery
- **Classic** (default) - Windows 10/11 aesthetic
- **Minimal** (default) - Subtle, low-profile icons
- **Neon City** - Vibrant neon aesthetic ([example](./examples/neon-city-icons))
- **Coral Reef** - Ocean-inspired pastel colors
- **Arctic Monochrome** - Clean black & white
- **Sunset Gradient** - Warm gradient colors
- And more...

## Client Libraries

### TypeScript (Electron)

```typescript
import { MarketplaceClient } from './client/MarketplaceClient';

// Browse and install plugins
const { items: plugins } = await MarketplaceClient.getPlugins({
  category: 'productivity',
  sort: 'popular',
});

await MarketplaceClient.installPlugin('conventional-commits', (progress) => {
  console.log(`Downloaded ${progress.percent}%`);
});

// Browse and install icon packs
const { items: packs } = await MarketplaceClient.getIconPacks({
  style: 'neon',
});

await MarketplaceClient.installIconPack('neon-city');
```

See [client/MarketplaceClient.ts](./client/MarketplaceClient.ts)

### C++ (Windows)

```cpp
#include "IconPackDownloader.h"

using namespace GitScribe;

// Download and install icon pack
bool success = IconPackDownloader::Install(
    "neon-city",
    IconPackDownloader::GetIconPacksDirectory(),
    [](size_t downloaded, size_t total) {
        printf("Downloaded %zu / %zu bytes\n", downloaded, total);
    }
);
```

See [client/IconPackDownloader.h](./client/IconPackDownloader.h)

## Repository Structure

```
gitscribe-plugins/
├─ MARKETPLACE.md              # Complete marketplace specification
├─ INTEGRATION.md              # Landing page integration guide
├─ CLAUDE.md                   # Development guidelines
│
├─ api/
│  └─ marketplace.openapi.yaml # OpenAPI 3.0 specification
│
├─ schemas/
│  ├─ plugin-manifest.schema.json       # JSON schema for plugin manifest
│  ├─ icon-pack-manifest.schema.json    # JSON schema for icon pack manifest
│  └─ marketplace-plugin.schema.json    # JSON schema for marketplace metadata
│
├─ examples/
│  ├─ conventional-commits/    # Example plugin
│  └─ neon-city-icons/         # Example icon pack
│
└─ client/
   ├─ IconPackDownloader.h     # C++ client (GitScribe Status)
   ├─ IconPackDownloader.cpp
   └─ MarketplaceClient.ts     # TypeScript client (GitScribe Full)
```

## Contributing

We welcome contributions!

### Submit a Plugin
1. Develop plugin following guidelines
2. Test with GitScribe Full
3. Submit via marketplace UI or PR
4. Wait for review (2-3 days)

### Submit an Icon Pack
1. Create 6 icons (16x16 + 32x32 .ico files)
2. Follow icon pack manifest schema
3. Submit via marketplace UI
4. Quick review (1-2 days)

### Improve Documentation
- Fix typos, add examples, clarify instructions
- Submit PRs to this repository

See CONTRIBUTING.md for detailed guidelines.

## Resources

- **Documentation**: [MARKETPLACE.md](./MARKETPLACE.md) | [INTEGRATION.md](./INTEGRATION.md)
- **API Reference**: [marketplace.openapi.yaml](./api/marketplace.openapi.yaml)
- **Examples**: [examples/](./examples/)
- **Schemas**: [schemas/](./schemas/)

## License

MIT License - See LICENSE.md

This SDK is permissively licensed to encourage a healthy plugin ecosystem.
