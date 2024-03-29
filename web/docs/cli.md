---
sidebar_position: 4
---

# CLI

## `-c`: path to the configuration file

See [the configuration section](./configuration.md) for more details.
Defaults to no configuration.
Example:

```bash
klunok -c ~/.config/klunok/config.lua
```

## `-w`: path to a directory that should be monitored for edited files

Can be specified more than once.
Files in subdirectories, subsubdirectories, etc. are also included.
The directory will be bind-mounted to itself if it's not already a mount point,
a detailed explanation of the mechanism is in
[the mounts section](./mounts.md).
Defaults to the current working directory.
Example:

```bash
klunok -w /home/nazar -w /etc/nixos
```

## `-e`: path to a directory that contains executable files

Klunok can recognize a text editor, office suite, vector graphics editor, and so on
only if the executable file of the text editor (office suite, …)
is within one of these directories (including subdirectories, subsubdirectories, …).

Can be specified more than once.
The directory will be bind-mounted to itself if it's not already a mount point,
a detailed explanation of the mechanism is in
[the mounts section](./mounts.md).

Defaults to `/` and, if you have
[installed Klunok with the Nix package manager](./installation.md?method=nix), `/nix/store`.

Usually, Linux distributions place all the executable files in several well-known
directories, such as `/bin` or `/usr/bin`.
If you know for sure which directories contain executable files of text editors
(office suites, …), it's a good security practice to list them explicitly, as in the example:

```bash
klunok -e /usr/bin -e /nix/store
```

## `-d`: path to a file or directory, which owner's identity will be used for running Klunok

Even though Klunok must be started as root to properly initialize,
Klunok will not continue running as root after initializing.
Instead, it will assume identity of the owner of the provided file or directory.
This way, most of the code of Klunok doesn't run as root, which is a good security practice.

Defaults to the current working directory.
Example:

```bash
klunok -d /home/nazar
```

## `-h`: print help

Example:

```bash
klunok -h
```

```
Klunok 0.1.1, Lua 5.4.6, compiled with Nix support
Usage: klunok [-h | -v | -c PATH | -d PATH | -w PATH | -e PATH]...
```

## `-v`: print version

Example:

```bash
klunok -v
```

```
Klunok 0.1.1, Lua 5.4.6, compiled with Nix support
```
