# gitscribe-core

Rust core library for GitScribe. Provides high-performance Git operations with a C API for shell extensions and Node bindings for Electron.

## Features

- **Fast**: Async operations built on libgit2
- **Cached**: SQLite-based status cache for instant overlay updates
- **Safe**: Memory-safe Rust prevents crashes
- **Cross-language**: C API + Node native module

## Architecture

```
gitscribe-core/
├── src/
│   ├── git/          # Git operations wrapper
│   ├── cache/        # Status caching layer
│   ├── queue/        # Operation queue
│   ├── ffi/          # C API for shell extension
│   └── node/         # Node native bindings
├── include/          # C headers
└── tests/            # Integration tests
```

## Building

### Prerequisites

- Rust 1.70+
- libgit2 1.7+
- SQLite 3.35+

### Build Rust library

```bash
cargo build --release
```

### Build C library

```bash
cargo build --release --features capi
```

This produces `gitscribe_core.dll` and `gitscribe_core.h`

### Build Node module

```bash
npm install
npm run build
```

## Usage

### From Rust

```rust
use gitscribe_core::{Repository, StatusCache};

let repo = Repository::open("C:/path/to/repo")?;
let status = repo.status_all()?;
```

### From C

```c
#include "gitscribe_core.h"

gs_repository* repo = gs_repository_open("C:/path/to/repo");
gs_status* status = gs_repository_status(repo);
gs_repository_free(repo);
```

### From Node

```javascript
const { Repository } = require('gitscribe-core');

const repo = new Repository('C:/path/to/repo');
const status = await repo.getStatus();
```

## Performance Targets

- Status check: <10ms (cached), <100ms (uncached)
- Commit operation: <50ms
- Push/Pull: Network-limited
- Overlay icon update: <50ms

## Caching Strategy

- SQLite database stores file status
- Filesystem watcher invalidates cache
- Partial updates for large repositories
- TTL-based expiration (configurable)

## Testing

```bash
cargo test
cargo test --features capi
npm test  # Node bindings
```

## Benchmarks

```bash
cargo bench
```

## License

Mozilla Public License 2.0 (MPL-2.0) - See LICENSE.md

This allows:
- ✅ Use in proprietary software
- ✅ Dynamic linking without license propagation
- ✅ Modifications must be shared under MPL-2.0
- ✅ Patent grant included

## Attribution

If you use GitScribe Core in your project, please include this attribution in your about/credits section:

```
Built with GitScribe Core (https://github.com/KyleEdwardDonaldson/GitScribe)
Copyright © 2025 GitScribe
```

This helps support the project and lets others discover it. Thank you!

## Contributing

This is the core of GitScribe. Contributions welcome, especially:

- Performance improvements
- Git operation coverage
- Cross-platform support (future)
- Bug fixes

Please open an issue before major PRs.
