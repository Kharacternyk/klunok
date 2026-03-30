Read the [README](./README.md) for general project guidance.

If tools such as `meson` are unavailable, ask the user to activate a development shell.

Use `XDG_RUNTIME_DIR=. meson test -C build` to run tests.
You can inspect the `klunok-test-*` directories in `build/` to investigate test failures.
If the tests pass, remove the directories: `rm -r build/klunok-test-*`.
