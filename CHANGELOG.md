# Changelog

All notable changes to **defer.h** will be documented here.

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).  
Versioning follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

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
- `DEFER_UNLOCK(mtx)` — `pthread_mutex_unlock()`
- Compiler detection: GCC / Clang via `__attribute__((cleanup))`
- `DEFER_SUPPORTED` compile-time guard with `#warning` on unsupported compilers
- MIT License
- Full README with before/after examples
- Examples: `files.c`, `memory.c`, `mutex.c`
- Tests: basic scope, nested scope, early return, NULL-safety
- GitHub Actions CI: GCC + Clang + ARM cross-compile

---

[Unreleased]: https://github.com/YOUR_USERNAME/defer.h/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/YOUR_USERNAME/defer.h/releases/tag/v0.1.0
