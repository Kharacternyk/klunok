# <img src='./misc/logo.svg' alt="Klunok logotype — a blue-yellow floppy disk with a trident" height="32"/> Klunok

Klunok is a smart versioning and automatic backup daemon for Linux.
It keeps a versioned history of files that you edit,
doing so in the background without any effort required from you.
It picks up everything that matters (sources, …)
and nothing that doesn't (binaries, …) automatically.

For the user guide, visit https://klunok.org, a rendered version of [`web/`](./web/).

The rest of this README is a developer guide.

## C Code Style

The C code is divided into "components".
Each component consists of a header in `inc/`,
an implementation in `src/` and a test in `tests/`.

The header of a component is either a namespace of related functions
or a namespace of related class-like APIs: a constructor
(`struct X *create_X()`, alternatively `load_X` or `open_X` if construction
does not consist purely of in-memory operations),
some methods (`do_Y(struct X *x)`),
and a destructor (`free_X(struct X *x)`).
Names of constructors and destructors contain the name of the class,
whereas methods generally should not.
If there's a "whole" and a "part" class within one component,
the constructor and destructor of the "part" class might not be exposed;
instances of the "part" class are obtained through some methods of the "whole" class instead.
Component headers do not include each other and therefore do not contain include guards.
If a header needs to reference a `struct` from another header,
the `struct` is just redeclared (`struct trace;`).

For error handling in the project's own code,
only the `trace` component is used, not return values or `errno`.
`TNEG` and `TNULL` macros are used for interfacing with third-party code
that uses `errno`.
`TNEG` and `TNULL` do not execute wrapped code if `!ok(trace)`,
otherwise the wrapped code is executed and `errno` is thrown into the `trace`
if the returned value is negative (`TNEG`) or `NULL` (`TNULL`).
Each function that accepts a `trace` must be a no-op if the `trace` is already `!ok`.
This allows callers to skip `if (ok(trace))` before calling the function.
Being a no-op implies no memory dereferencing since the arguments can be `NULL`,
no changes to objects in memory or files in the filesystem,
no operations on file descriptors since they can be invalid, etc.
Functions in the `trace` component like `catch_static` are exceptions to this
no-op rule, of course.

Arguments are ordered so that the most "interesting" ones are to the left
and the most "boring" ones are to the right.
For example,
`concat_string(const char *string, struct buffer *buffer, struct trace *trace)`
is ordered this way because one `trace` is reused throughout most of the program,
and one `buffer` could be reused in a chain of `concat_*` calls
that builds a filesystem path.
Therefore, the most interesting argument is `string` because we can usually
guess from context what is going to be passed as `buffer` and `trace`.

Macros are generally used only for things that are impossible to implement without them
(`TNEG` and `TNULL`) or for compile-time string concatenation (testing code mostly).
Otherwise functions and `static const` variables are preferred.

`clang-format` with the default configuration is used for formatting.

## High-level Flow From a Security Perspective

See the relevant [section of the user guide](./web/docs/security.md).

## Important Components

### Low-level

- `buffer`: a growable string that knows its size and has lazy hashing.
- `set`: a multi-set of `buffer`s.
- `list`: a linked list of strings.
- `messages`: static error messages that can be caught with `catch_static`.
- `params`: CLI parsing.
- `parents`: create and remove parent directories, find common parent directories.
- `sync`: copy files and directories.
- `trace`: error handling, discussed above.

### High-level

- `config`: shared configuration interface with both a Lua
  (`config-lua.c`, [`lua/`](./lua/))
  and a compile-time (`config-static.c`) implementation.
- `handler`: most of the high-level logic.
- `linq`: an implementation of a filesystem-backed debouncing queue of paths with metadata.

## Tests

1. `meson setup build`
2. `meson test -C build`

Tests are run with Valgrind if available,
and must free all allocated memory to pass in this case.
Testing with Valgrind is preferred.

To test with different Lua versions, as well as without Lua,
run `nix flake check` (slow).
