Read the [README](./README.md) for general project guidance.

If tools such as `meson` are unavailable, ask the user to activate a development shell.

Use `XDG_RUNTIME_DIR=. meson test -C build` to run tests,
but always enumerate relevant tests explicitly,
for example: `XDG_RUNTIME_DIR=. meson test -C build config-lua handler linq`.
You can inspect the `klunok-test-*` directories in `build/` to investigate test failures.
If the tests pass, remove the directories: `rm -r build/klunok-test-*`.

In prose, try to keep each sentence on its own line, unless the sentence is very short.
If a sentence is longer than 93 characters, hard wrap it.
Prefer line breaks at punctuation.
Prefer line breaks that don't introduce a very short line.

Never silently overwrite changes that have been made to files since you last read them.
Ask for guidance instead.
Most likely, a human made the changes for a reason.
