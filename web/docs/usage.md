---
sidebar_position: 3
---

import Asciinema from '../src/asciinema.jsx';

# Usage

Klunok uses the Linux kernel fanotify API, which as of now requires root privileges.
For this reason, Klunok should be invoked, for example, with `sudo`:

```bash
sudo -b klunok
```

The `-b` option of `sudo` makes the command run in the background,
which lets you continue using the terminal for other commands.

If Klunok is invoked without the necessary privileges, you will face an error:

```
Cannot initialize fanotify
└─┤because of│ Operation not permitted
```

Klunok will drop privileges to the owner of the current working directory
after initializing the fanotify API.
The complete picture of security measures and implications is described in
[the security section](./advanced/security.md).

Once Klunok successfully starts,
it watches for file edits in the current working directory and its descendants.
When it thinks that the current version of a file is more or less stable
(the file hasn't been edited for one minute),
it copies the file to the `klunok/store` directory.

Here is a simple demo:

<Asciinema src="/demo.cast">
  <pre>
$ ls
$ sudo -b klunok
$ ls
klunok/
$ ls klunok
var/
$ nano hello.txt
$ ls
hello.txt  klunok/
$ ls klunok
var/
$ sleep 60
$ ls klunok
store/  var/
$ cat klunok/store/hello.txt/v2023-06-17-15-07.txt
Hello, World!
  </pre>
</Asciinema>
