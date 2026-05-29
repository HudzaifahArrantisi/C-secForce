# Repository Guidelines

## Project Structure & Module Organization

CyberForce is a C security-testing framework. The main application lives under `src/`: `src/main.c` handles CLI startup, `src/core/` contains orchestration, threading, rate limiting, and protocol routing, `src/modules/` contains protocol brute-force modules, and `src/utils/` holds logging, wordlist, proxy, pattern, and crypto helpers. Public headers are in `src/include/`. Runtime data and sample lists are in `data/wordlists/`, while `examples/` contains sample configuration and scripts. `simple_cyberforce.c` is the standalone build path used when external libraries are unavailable.

## Build, Test, and Development Commands

- `.\build.ps1 build` builds the full Windows executable with libcurl, libssh, OpenSSL, Winsock, and pthread support.
- `.\build.ps1 build simple` builds the standalone executable from `simple_cyberforce.c`.
- `.\build.ps1 run` builds if needed, then runs `cyberforce.exe --help`.
- `.\build.ps1 clean` removes local build outputs.
- `mingw32-make all` or `make all` builds through the Makefile in MSYS2/Linux.
- `make debug`, `make release`, `make analyze`, and `make docs` provide debug, optimized, static-analysis, and Doxygen workflows.

The Makefile defines `make test`, but `tests/run_tests.sh` is not currently present. Add the test runner before relying on that target.

## Coding Style & Naming Conventions

Use C with 4-space indentation, descriptive snake_case names, and module-specific prefixes where helpful, such as `http_`, `ssh_`, `rate_limiter_`, or `thread_manager_`. Keep declarations in `src/include/` synchronized with implementations. Preserve existing conditional compilation patterns for Windows and POSIX support (`#ifdef _WIN32`). Run `make format` when `clang-format` is available; otherwise match the surrounding style manually.

## Testing Guidelines

Prefer small, reproducible tests for protocol parsing, wordlist loading, rate limiting, and error handling. Place future tests under `tests/` and name shell runners or fixtures by behavior, for example `test_rate_limiter.sh` or `http_auth_fixture.*`. For now, validate changes with the narrowest relevant build command and run `.\cyberforce.exe --help` or `.\build.ps1 run` after successful compilation.

## Commit & Pull Request Guidelines

No commit history convention was available during this edit, so use concise imperative messages such as `Fix Windows build script` or `Add HTTP response matcher tests`. Pull requests should include a short purpose statement, commands run, target platform, dependency changes, and screenshots or terminal output when CLI behavior changes.

## Security & Configuration Tips

Use this tool only for authorized testing. Do not commit real credentials, private targets, generated logs, or output files. Keep large custom wordlists outside the repository unless they are sanitized examples.
