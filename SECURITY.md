# Security Policy

## Scope

`defer.h` is a compile-time macro library with no runtime registry and no heap
allocation inside the library itself.

Realistic security concerns:

- Macro expansion producing undefined behavior
- Incorrect cleanup order causing use-after-free
- Incompatible callback signatures in `DEFER(fn, ctx)`

## Supported Versions

| Version | Supported |
|---|---|
| `1.0.x` (latest) | Yes |
| older | Update to latest |

## Reporting a Vulnerability

Do not open a public issue for a security vulnerability.

Use GitHub's private Security Advisory flow:

https://github.com/Vanderhell/defer/security/advisories/new

Include:

- Description of the issue
- Affected compiler(s) and platform(s)
- Minimal reproducer if possible
- Potential impact

## Known Limitations

- MSVC is unsupported unless `DEFER_ALLOW_NOOP_FALLBACK` is explicitly
  defined.
- Cleanup order is LIFO by design.
- `DEFER(fn, ctx)` requires a compatible `void (*)(void *)` callback.
- pthread support requires explicit `DEFER_WITH_PTHREAD` before including
  `defer.h`.
