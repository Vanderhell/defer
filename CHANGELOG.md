# Changelog

All notable changes to `defer.h` are documented here.

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
Versioning follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Changed
- Public header now uses explicit opt-ins for helper macros that need hosted or POSIX headers.
- Added named guards with explicit dismissal for manual cleanup patterns.
- Tightened callback typing and capture semantics for deferred cleanup.
- Updated examples, tests, CMake, Makefile, and CI scaffolding to match the current contract.

---

## [1.0.0] - 2026-03-08

### Added
- `DEFER(fn, ctx)` - deferred callback on scope exit.
- `DEFER_FREE(ptr)` - opt-in helper for deferred `free()`.
- `DEFER_FCLOSE(fp)` - opt-in helper for deferred `fclose()`.
- `DEFER_CLOSE(fd)` - opt-in helper for deferred `close()`.
- `DEFER_UNLOCK(mtx)` - opt-in helper for deferred `pthread_mutex_unlock()`.
- `DEFER_SUPPORTED` compile-time feature detection.
- Optional no-op fallback for unsupported compilers via `DEFER_ALLOW_NOOP_FALLBACK`.
- Examples for memory, file, and mutex usage.
- Initial test suite and GitHub Actions CI.

[Unreleased]: https://github.com/Vanderhell/defer/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/Vanderhell/defer/releases/tag/v1.0.0
