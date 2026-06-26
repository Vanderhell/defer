# Verification

## v2.0.0 WSL Matrix

- `make`, `make test`, `make examples`: OK
- GCC Debug/Release CTest: 7/7
- Clang Debug/Release CTest: 7/7
- Direct GCC C99/C11 O0/O2: 18/18
- ASan / UBSan / LSan: 7/7
- Valgrind: 0 errors, 0 leaks
- `clang --analyze`: OK
- `clang-tidy`: OK
- `cppcheck defer.h`: OK
- ARM Cortex-M0/M4 compile-only: OK
- install + `find_package` consumer: OK
- `git diff --check`: OK
