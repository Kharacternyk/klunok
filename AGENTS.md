If tools such as `meson` are unavailable,
install them only when running in the cloud on Ubuntu.
Otherwise, ask the user to activate a development shell.

Use `XDG_RUNTIME_DIR=. meson test -C build` to run tests.
You can inspect the `klunok-test-*` directories in `build/` to understand test failures.
If tests pass, remove the directories: `rm -r build/klunok-test-*`.
