---
sidebar_position: 8
---

# Security

## Overview

1. Klunok starts as root.
2. Klunok parses command-line parameters.
3. Klunok initializes a fanotify API handle.
4. Klunok performs [bind mounts](./mounts.md) if necessary.
5. Klunok [drops privileges](./cli.md#-d-path-to-a-file-or-directory-which-owners-identity-will-be-used-for-running-klunok), exits if cannot drop.
6. Klunok parses [Lua configuration](./configuration.md).
7. Klunok listens for fanotify events.

The only sensitive thing that is held by Klunok after it drops privileges is
the stream of fanotify events.
The events contain read-only open file descriptors of edited files
and executable files of newly started applications.
Therefore, Klunok monitors only the current working directory by default.
This avoids receiving read-only file descriptors of, for example, `/etc/shadow`.

## Best practices

- Avoid `klunok -w /`.
- Do use [the `-e` command-line option](./cli.md#-e-path-to-a-directory-that-contains-executable-files).

## Security policy

Please see https://github.com/Kharacternyk/klunok/blob/master/SECURITY.md.

## Static binary reproducibility

You can check that the distributed binary
has been built from the source without modifications
by reproducing the build locally with [Nix](https://nixos.org/).
For example, let's verify that the `v0.1.1` release has not been tampered with:

```bash
nix build github:Kharacternyk/klunok/v0.1.1#static
curl -Lo binary https://github.com/Kharacternyk/klunok/releases/download/v0.1.1/klunok
cmp binary ./result/bin/klunok
```

The output of `cmp` must be empty.
