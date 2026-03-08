# Security Policy

## Scope

**defer.h** is a compile-time macro library with zero runtime overhead and no dynamic allocation.  
The attack surface is minimal by design — there is no network code, no parsing, no heap usage.

Realistic security concerns:

- Macro expansion producing undefined behavior
- Incorrect cleanup order causing use-after-free
- Unsafe casting in `DEFER(fn, ctx)` with wrong function signature

---

## Supported Versions

| Version | Supported |
|---|---|
| `0.1.x` (latest) | ✅ |
| older | ❌ — update to latest |

---

## Reporting a Vulnerability

Do **not** open a public GitHub issue for security vulnerabilities.

Instead, use GitHub's private **Security Advisory** feature:  
`https://github.com/YOUR_USERNAME/defer.h/security/advisories/new`

Include:
- Description of the issue
- Affected compiler(s) and platform(s)
- Minimal reproducer if possible
- Potential impact

You will get a response within **7 days**.  
If confirmed, a fix will be released within **30 days** with full credit to the reporter.

---

## Known Limitations (not vulnerabilities)

- MSVC is unsupported — `DEFER_SUPPORTED` will be `0` and macros are undefined by default to prevent silent failure. Use `DEFER_ALLOW_NOOP_FALLBACK` only if you fully understand the risks.
- Cleanup order is LIFO (last declared, first cleaned) — this is intentional and matches RAII semantics
- `DEFER(fn, ctx)` uses a `void*` cast — caller is responsible for correct function signature
- pthread support requires explicit `DEFER_WITH_PTHREAD` before including `defer.h`
