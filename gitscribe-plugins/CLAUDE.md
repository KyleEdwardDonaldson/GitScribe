# gitscribe-plugins - Development Guide

## Purpose

The plugin SDK is critical for building an ecosystem. It should:
- **Enable innovation**: Let community build features we haven't thought of
- **Be simple**: Getting started should take <5 minutes
- **Be safe**: Bad plugins can't harm users or their data
- **Be powerful**: Enable real, meaningful extensions

## Architecture

### Sandboxed Execution

Plugins run in a secure sandbox (QuickJS or WASM):

```
┌─────────────────────────────────────┐
│         GitScribe Main App          │
│  ┌────────────────────────────────┐ │
│  │     Plugin Runtime (QuickJS)   │ │
│  │  ┌──────────┐  ┌──────────┐   │ │
│  │  │ Plugin A │  │ Plugin B │   │ │
│  │  └──────────┘  └──────────┘   │ │
│  └────────────────────────────────┘ │
│              ↕ IPC                  │
│  ┌────────────────────────────────┐ │
│  │         Plugin Host            │ │
│  │   (Manages lifecycle, API)     │ │
│  └────────────────────────────────┘ │
└─────────────────────────────────────┘
```

### Why QuickJS?

- **Lightweight**: <1MB runtime
- **Sandboxed**: No file/network access by default
- **Fast**: Sufficient for most plugin use cases
- **Embeddable**: Easy to integrate

### Why WASM?

- **Performance**: For compute-heavy plugins
- **Language agnostic**: Rust, Go, C++, etc.
- **Secure**: Runs in sandbox by design

## API Design Principles

1. **Async by default**: All API calls return Promises
2. **Type-safe**: Full TypeScript definitions
3. **Fail-safe**: Errors don't crash GitScribe
4. **Versioned**: Semantic versioning for API changes
5. **Documented**: Every API has examples

## Plugin Lifecycle

```
┌─────────────┐
│   Install   │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Enable    │
└──────┬──────┘
       │
       ▼
┌─────────────┐    ┌──────────────────┐
│  Activate   │───→│ onActivate()     │
└──────┬──────┘    └──────────────────┘
       │
       │ (Plugin runs)
       │
       ▼
┌─────────────┐    ┌──────────────────┐
│ Deactivate  │───→│ onDeactivate()   │
└──────┬──────┘    └──────────────────┘
       │
       ▼
┌─────────────┐
│   Disable   │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Uninstall  │
└─────────────┘
```

## Implementing the Plugin Host

### Loading Plugins

```typescript
// plugin-host/loader.ts
import { QuickJSRuntime } from 'quickjs-emscripten';

export class PluginLoader {
  private runtime: QuickJSRuntime;

  async loadPlugin(path: string): Promise<Plugin> {
    // Read plugin code
    const code = await fs.readFile(path, 'utf-8');

    // Create sandbox
    const vm = this.runtime.newContext();

    // Inject API
    this.injectAPI(vm);

    // Execute plugin code
    const result = vm.evalCode(code);

    // Extract plugin instance
    const plugin = this.extractPlugin(result);

    return plugin;
  }

  private injectAPI(vm: QuickJSVm) {
    // Expose GitAPI
    vm.setProp(vm.global, 'GitAPI', vm.newObject());
    // ... expose methods

    // Expose UI
    vm.setProp(vm.global, 'UI', vm.newObject());
    // ... expose methods
  }
}
```

### API Bridging

```typescript
// Bridge between QuickJS and main process
class APIBridge {
  constructor(private ipcChannel: IpcChannel) {}

  async call(method: string, ...args: any[]): Promise<any> {
    // Send IPC message to main process
    const result = await this.ipcChannel.invoke('plugin:api', method, args);
    return result;
  }
}

// Expose to plugin
vm.newFunction('callGitAPI', (method, ...args) => {
  return bridge.call(method, ...args);
});
```

## Security Model

### Permission System

```typescript
interface PluginManifest {
  name: string;
  version: string;
  permissions: Permission[];
}

enum Permission {
  NETWORK = 'network',
  FILESYSTEM_READ = 'filesystem:read',
  FILESYSTEM_WRITE = 'filesystem:write',
  SHELL_EXECUTE = 'shell:execute',
  CLIPBOARD = 'clipboard',
}

// Check before executing privileged operations
function checkPermission(plugin: Plugin, permission: Permission) {
  if (!plugin.manifest.permissions.includes(permission)) {
    throw new Error(`Plugin ${plugin.name} lacks permission: ${permission}`);
  }
}
```

### Resource Limits

```typescript
interface ResourceLimits {
  maxMemory: number;      // 50MB default
  maxCpuTime: number;     // 1000ms per operation
  maxFileSize: number;    // 10MB per file read
  maxNetworkRequests: number; // 10 per minute
}

// Enforce limits
class PluginRuntime {
  private limits: ResourceLimits;

  async executePlugin(plugin: Plugin) {
    const startTime = Date.now();
    const startMemory = process.memoryUsage().heapUsed;

    try {
      await plugin.execute();
    } finally {
      const elapsed = Date.now() - startTime;
      const memoryUsed = process.memoryUsage().heapUsed - startMemory;

      if (elapsed > this.limits.maxCpuTime) {
        this.terminatePlugin(plugin, 'CPU time limit exceeded');
      }
      if (memoryUsed > this.limits.maxMemory) {
        this.terminatePlugin(plugin, 'Memory limit exceeded');
      }
    }
  }
}
```

## API Implementation

### Git Operations

```typescript
// Exposed to plugins
export const GitAPI = {
  async getStatus(repoPath: string): Promise<FileStatus[]> {
    // Validate repoPath is within allowed scope
    validatePath(repoPath);

    // Call gitscribe-core
    const status = await core.getStatus(repoPath);

    return status;
  },

  async commit(repoPath: string, message: string): Promise<void> {
    validatePath(repoPath);
    validateCommitMessage(message);

    await core.commit(repoPath, message);
  }
};
```

### UI Operations

```typescript
export const UI = {
  showNotification(message: string, type: NotificationType) {
    // Send to renderer via IPC
    ipcRenderer.send('ui:notification', { message, type });
  },

  async showDialog(options: DialogOptions): Promise<DialogResult> {
    return await ipcRenderer.invoke('ui:dialog', options);
  },

  createPanel(id: string, title: string, component: string): Panel {
    // component is serialized React component or HTML
    const panel = new Panel(id, title, component);
    ipcRenderer.send('ui:create-panel', panel.serialize());
    return panel;
  }
};
```

## Plugin Development Experience

### CLI Tool

```bash
# Create plugin from template
npm create gitscribe-plugin

# Test plugin locally
gitscribe plugin dev ./my-plugin

# Package for distribution
gitscribe plugin build

# Publish to registry
gitscribe plugin publish
```

### Hot Reload

```typescript
// Watch for file changes
chokidar.watch('./src').on('change', async (path) => {
  console.log('Reloading plugin...');
  await pluginHost.reload(pluginId);
  console.log('Plugin reloaded');
});
```

### Developer Tools

- **Plugin Inspector**: View plugin state, permissions, logs
- **API Explorer**: Interactive API testing
- **Performance Profiler**: See where time is spent
- **Debug Console**: `console.log` from plugins

## Plugin Registry

### Submission Process

1. Developer creates plugin
2. Runs `gitscribe plugin publish`
3. Plugin submitted to review queue
4. Automated checks:
   - No malicious code patterns
   - Declares all permissions
   - Passes automated tests
5. Manual review (for high-risk permissions)
6. Approved → Published to registry

### Versioning

Follow semantic versioning:
- **Major**: Breaking API changes
- **Minor**: New features, backward compatible
- **Patch**: Bug fixes

### Discovery

Registry UI shows:
- Featured plugins
- Most popular
- Recently updated
- Search by category

## Testing

### Unit Tests

```typescript
import { createMockPlugin } from '@gitscribe/plugin-test-utils';

describe('MyPlugin', () => {
  it('registers context menu', async () => {
    const plugin = createMockPlugin(MyPlugin);
    await plugin.onActivate();

    expect(plugin.contextMenu.items).toHaveLength(1);
    expect(plugin.contextMenu.items[0].id).toBe('my-action');
  });
});
```

### Integration Tests

```typescript
import { GitScribeTestHarness } from '@gitscribe/plugin-test-utils';

describe('MyPlugin Integration', () => {
  it('formats commit messages', async () => {
    const harness = await GitScribeTestHarness.create();
    await harness.loadPlugin('./my-plugin');

    const result = await harness.triggerCommit('test repo', 'bad message');

    expect(result.message).toBe('feat: bad message');
  });
});
```

## Performance Considerations

### Lazy Loading

Only load plugins when needed:

```typescript
class PluginManager {
  private plugins = new Map<string, Plugin>();
  private loaded = new Set<string>();

  async activatePlugin(id: string) {
    if (!this.loaded.has(id)) {
      const plugin = await this.loadPlugin(id);
      this.plugins.set(id, plugin);
      this.loaded.add(id);
    }

    await this.plugins.get(id)!.onActivate();
  }
}
```

### Caching

Cache expensive operations:

```typescript
class PluginCache {
  private cache = new Map<string, any>();

  async getOrCompute<T>(key: string, fn: () => Promise<T>): Promise<T> {
    if (this.cache.has(key)) {
      return this.cache.get(key);
    }

    const value = await fn();
    this.cache.set(key, value);
    return value;
  }
}
```

## Error Handling

Plugins shouldn't crash GitScribe:

```typescript
async function executePluginSafely(plugin: Plugin, method: string, ...args: any[]) {
  try {
    return await plugin[method](...args);
  } catch (error) {
    console.error(`Plugin ${plugin.name} error:`, error);
    UI.showNotification(`Plugin ${plugin.name} encountered an error`, 'error');

    // Optionally disable plugin
    if (isCriticalError(error)) {
      await pluginManager.disablePlugin(plugin.id);
    }

    return null;
  }
}
```

## Future Enhancements

- [ ] Plugin marketplace with reviews and ratings
- [ ] Paid plugins (revenue share with developers)
- [ ] Enterprise plugin distribution
- [ ] Plugin templates for common use cases
- [ ] Visual plugin builder (no-code)
- [ ] Cross-platform plugins (when GitScribe goes cross-platform)
