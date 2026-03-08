# Contributing to defer.h

First off — thank you. This is a small project with a focused scope.  
Keep that scope in mind before opening a PR.

---

## Philosophy

- **Single header.** No build system required. Ever.
- **Zero allocation.** No `malloc` inside the library itself.
- **Portable.** If it doesn't work on GCC ARM, it doesn't ship.
- **Minimal API surface.** Less is more.

If your contribution grows the API significantly, open an issue first to discuss.

---

## How to contribute

### Bug reports

Open an issue with:
- Compiler + version (`gcc --version`)
- Platform / architecture (x86-64, ARM Cortex-M, AVR, ...)
- Minimal reproducer (20 lines max)
- Expected vs actual behavior

### Feature requests

Open an issue before writing code.  
Describe the real problem, not just the proposed solution.

### Pull requests

1. Fork the repo and create a branch: `git checkout -b fix/your-fix`
2. Make your change — keep it focused and small
3. Add or update tests in `tests/test_defer.c`
4. Verify locally:
   ```sh
   gcc -Wall -Wextra -o test tests/test_defer.c && ./test
   clang -Wall -Wextra -o test tests/test_defer.c && ./test
   ```
5. Open the PR — describe what and why, not just how

---

## What will be accepted

| Type | Likely accepted |
|---|---|
| Bug fix | ✅ Yes |
| New NULL-safe helper (DEFER_MUNMAP, ...) | ✅ If it's common enough |
| Compiler portability fix | ✅ Yes |
| Docs / README improvement | ✅ Yes |
| New abstraction layer on top | ❌ Out of scope |
| Dynamic defer stack / heap usage | ❌ Against philosophy |
| C++ support | ❌ Different project |

---

## Code style

- C99 minimum, C11 preferred
- 4 spaces, no tabs
- `snake_case` for everything
- Macros in `SCREAMING_SNAKE_CASE`
- Comments in English

---

## License

By contributing, you agree your code will be released under the [MIT License](LICENSE).
