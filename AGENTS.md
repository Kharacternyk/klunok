# Klunok Codebase Notes

## Purpose

Klunok is a C daemon that watches filesystem activity with `fanotify` and stores
versioned snapshots of edited files. The main binary is built from `src/*.c`.
There is an optional Lua-backed configuration layer and a static fallback config
backend when Lua is unavailable.

## High-Level Flow

1. `src/main.c` parses CLI arguments, initializes `fanotify`, discovers and
   marks watched mounts, drops privileges, and loads a `handler`.
2. `src/handler.c` is the orchestration layer:
   - classifies exec/write events
   - reloads config on config-file writes
   - pushes paths into the debouncing queue (`linq`)
   - drains the queue on timeout and stores snapshots
3. `src/linq.c` implements the on-disk debouncing queue as numbered symlinks.
4. `src/sync.c` copies regular files into the store and materializes project
   snapshots.
5. `src/journal.c` appends human-readable event records.

## Important Modules

- `src/trace.c`, `inc/trace.h`
  Custom try/catch/finally implementation. This is a core project invariant.
- `src/config-lua.c`
  Loads config from embedded pre/post Lua scripts plus a user file.
- `src/config-static.c`
  Static config fallback used when Lua is not present.
- `src/sieve.c`
  Matches absolute and relative paths against config-controlled sets.
- `src/set.c`
  Hash-set / multiset used throughout the codebase.
- `src/buffer.c`
  Growable string builder used for path and message construction.
- `src/parents.c`
  Creates and removes ancestor directories for generated paths.
- `src/elfinterp.c`
  Reads ELF interpreter paths from executable FDs.

## Trace Contract

The project relies heavily on this rule:

- Any ordinary function that accepts `struct trace *trace` must be a no-op when
  `!ok(trace)`.
- Callers often rely on that property instead of adding redundant
  `if (ok(trace))` guards.
- The macros `TNEG(...)` and `TNULL(...)` are the standard way to preserve this:
  they skip the wrapped operation when the trace is already failed.

Practical guidance:

- If you add a new trace-aware helper, start with `if (!ok(trace)) return ...;`
  unless every side effect is already protected by `TNEG`/`TNULL` or other
  trace-aware helpers.
- Do not perform raw filesystem or memory side effects before that contract is
  enforced.
- `throw_*`, `try`, `catch_static`, `finally*`, and `rethrow_context` are trace
  control primitives, not ordinary no-op helpers.

## Trace Semantics

- `try(trace)` increases `pre_throw_depth` when the trace is currently clean and
  `post_throw_depth` otherwise.
- `finally_rethrow_static(...)` wraps failures from inner scopes in a new static
  message.
- `rethrow_context(...)` adds dynamic context while unwinding.
- `catch_static(...)` only matches the top frame and only for static messages.
- `finally_catch_all(...)` clears the current trace state for the active try
  scope. Be careful: this is not a no-op operation.

## Storage / Queue Model

- The queue directory contains numbered symlinks whose targets encode both the
  original path and small metadata bits.
- The queue head is processed in `handle_timeout()` in `src/handler.c`.
- Regular file snapshots go under `store_root/<relative_path>/<version><ext>`.
- Project snapshots use `project_store_root` and an additional unstable working
  tree under `unstable_project_store_root`.
- History-path offsets are tracked via small files under `offset_store_root`.

## Configuration Model

- Lua config defaults are generated from `lua/config.lua.md`.
- `lua/weave.awk` extracts embedded `pre-config` and `post-config` Lua snippets.
- The static config backend is intended to mirror Lua defaults when Lua support
  is unavailable.
- Path-matching settings include:
  - `project_roots`
  - `project_parents`
  - `history_paths`
  - `excluded_paths`
  - `included_paths`
  - `cluded_paths`

## Testing

- Unit tests live in `tests/*.c` and are linked into a single test executable.
- `tests/main.c` dispatches a specific test function via `dlsym`.
- `tests/trace.c` is the main executable spec for trace semantics.
- There is a small fuzz target in `fuzz/elfinterp.c`.

## Editing Guidance

- Preserve the trace no-op contract when changing any trace-aware function.
- Be careful with file-descriptor cleanup on every failure path.
- Be careful with partial reads/writes and pointer arithmetic around raw I/O.
- `handler.c`, `linq.c`, and `sync.c` are the highest-risk files for subtle
  correctness regressions.
- When touching trace behavior, update or extend `tests/trace.c`.

## Known Sharp Edges

- `finally_catch_all()` intentionally clears trace state; misuse can swallow an
  existing failure.
- Queue and storage code encode metadata into symlink targets; small mistakes in
  offset math can corrupt behavior.
- The codebase depends on side-effect-free behavior after trace failure, so an
  innocent-looking raw `unlink`, `mkdir`, `write`, or similar call can violate
  core assumptions if it is not properly guarded.
