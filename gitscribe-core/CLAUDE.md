# gitscribe-core - Development Guide

## Purpose

This is the high-performance Rust core that powers all Git operations in GitScribe. It must be:
- **Fast**: Every operation impacts perceived performance
- **Reliable**: Crashes here crash the whole app
- **Safe**: Memory safety is non-negotiable
- **Well-tested**: This is critical infrastructure

## Architecture Layers

### 1. Git Operations Layer (`src/git/`)
Wraps libgit2 with async/await and error handling:
- Repository management
- Status queries
- Commit/stage/unstage
- Push/pull/fetch
- Branch operations
- Merge/rebase

### 2. Caching Layer (`src/cache/`)
SQLite-based cache for instant status queries:
- File status cache
- Commit metadata cache
- Filesystem watcher integration
- Invalidation strategies

### 3. Operation Queue (`src/queue/`)
Prevents "repository locked" errors:
- Queues concurrent operations
- Prioritizes user-initiated actions
- Background operations (fetch, etc.)

### 4. FFI Layers
- `src/ffi/`: C API for shell extension
- `src/node/`: Node N-API bindings for Electron

## Performance Principles

1. **Async Everything**: No blocking operations on main thread
2. **Cache Aggressively**: Disk is slow, memory is fast
3. **Lazy Load**: Don't compute what you don't need
4. **Batch Operations**: Minimize round-trips
5. **Profile Often**: Use `cargo flamegraph` regularly

## Error Handling

Use `anyhow::Result` internally, convert to specific error types at API boundaries:

```rust
// Internal
pub fn do_thing() -> anyhow::Result<()> { ... }

// C API
pub extern "C" fn gs_do_thing() -> i32 { ... }

// Node API
#[napi]
pub fn doThing() -> napi::Result<()> { ... }
```

## Testing Strategy

- **Unit tests**: Test individual functions
- **Integration tests**: Test against real Git repos
- **Benchmark tests**: Ensure performance targets met
- **Fuzz tests**: Catch edge cases (future)

## Common Tasks

### Adding a New Git Operation

1. Add to `src/git/operations.rs`
2. Add to C API in `src/ffi/`
3. Add to Node API in `src/node/`
4. Add integration test
5. Update benchmarks

### Optimizing a Slow Operation

1. Profile with `cargo flamegraph`
2. Check if caching can help
3. Consider batching
4. Benchmark before/after

### Debugging Crashes

1. Build with debug symbols: `cargo build`
2. Attach debugger to shell process
3. Enable trace logging: `GITSCRIBE_LOG=trace`

## Dependencies

- **libgit2**: Core Git operations
- **tokio**: Async runtime
- **rusqlite**: Status cache
- **notify**: Filesystem watching
- **napi-rs**: Node bindings
- **cbindgen**: C header generation

## Building for Production

```bash
# Optimize for size and speed
cargo build --release --features "capi,node"

# Enable LTO
RUSTFLAGS="-C lto=fat -C embed-bitcode=yes" cargo build --release
```

## Memory Management

- Rust code: Normal ownership rules
- C API: Caller must free with `gs_*_free()` functions
- Node API: Garbage collected automatically

## Performance Benchmarks

Run benchmarks:
```bash
cargo bench --features bench
```

Target benchmarks (on typical dev machine):
- `status_cached`: <10ms
- `status_uncached`: <100ms
- `commit_small`: <50ms
- `commit_large`: <200ms

## Cache Invalidation

Cache is invalidated when:
- File modification detected (filesystem watcher)
- Git operation performed via our API
- Manual cache clear
- TTL expires (default 5 minutes)

## Logging

Use `tracing` crate with structured logging:

```rust
use tracing::{info, warn, error, debug};

debug!(path = %path, "Checking status");
info!(files = count, "Status updated");
warn!(error = %e, "Cache miss");
error!(error = %e, "Operation failed");
```

Set log level: `GITSCRIBE_LOG=debug`

## Security Considerations

- Never log credentials
- Validate all paths (prevent directory traversal)
- Sanitize user input before passing to Git
- Use secure credential storage (Windows Credential Manager)

## Future Work

- Linux/macOS support (if demand exists)
- GPU-accelerated diff (experimental)
- Cloud caching for CI (enterprise feature)
- Distributed repository cache
