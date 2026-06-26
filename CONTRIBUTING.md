# Contributing to defer.h

This project stays intentionally small:

- single public header
- no heap allocation inside the library
- no runtime registry
- no C++ support claims

## Before you change code

- Read the existing tests, examples, and workflows.
- Keep changes focused.
- Do not weaken warnings, sanitizers, or negative tests to make the tree pass.

## What to include

- A code change that addresses the issue
- A test that proves the behavior
- Documentation updates if the contract changed

## Local checks

Run the strictest build path available in your environment. At minimum:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

If you are working in a Unix-like toolchain, also run the Makefile targets for
strict GCC and Clang builds plus sanitizers.

## Style

- C99 minimum
- ASCII unless the file already uses Unicode
- `snake_case` for functions and variables
- `DEFER_` for public macros

## Scope

If a change would add a runtime registry, heap-backed defer stack, or a C++
layer, stop and discuss the direction first.
