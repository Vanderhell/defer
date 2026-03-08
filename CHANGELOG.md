# Changelog

All notable changes to **defer.h** will be documented here.

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).  
Versioning follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Changed
- **Compiler fallback hardening:** Unsupported compilers no longer silently define no-op macros by default
- **Explicit opt-in for no-op fallback:** `DEFER_ALLOW_NOOP_FALLBACK` macro required for no-op behavior on unsupported compilers
- **Explicit pthread support:** `DEFER_UNLOCK` available only when `DEFER_WITH_PTHREAD` is defined before including `defer.h`
- **Portability improvement:** Replaced `__builtin_free` with standard `free()` for better compatibility
- **C99 compliance:** Removed C11-only constructs; library remains C99-compatible

### Planned
- IAR compiler support investigation
- DEFER_LOCK / DEFER_UNLOCK pair macro
- CMake FetchContent support

---

## [0.1.0] - 2025-XX-XX

### Added
- `DEFER(fn, ctx)` — generic deferred call with context pointer
- `DEFER_FREE(ptr)` — NULL-safe `free()`
- `DEFER_FCLOSE(fp)` — NULL-safe `fclose()`
- `DEFER_CLOSE(fd)` — `close()` for file descriptors ≥ 0
- `DEFER_UNLOCK(mtx)` — `pthread_mutex_unlock()` (requires `DEFER_WITH_PTHREAD`)
- Compiler detection: GCC / Clang via `__attribute__((cleanup))`
- `DEFER_SUPPORTED` compile-time constant to detect support status
- Safe fallback for unsupported compilers: macros undefined unless `DEFER_ALLOW_NOOP_FALLBACK` is set
- MIT License
- Full README with before/after examples
- Examples: `files.c`, `memory.c`, `mutex.c`
- Tests: basic scope, nested scope, early return, NULL-safety
- GitHub Actions CI: GCC + Clang + ARM cross-compile

---

[Unreleased]: https://github.com/YOUR_USERNAME/defer.h/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/YOUR_USERNAME/defer.h/releases/tag/v0.1.0
