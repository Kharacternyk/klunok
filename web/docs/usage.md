---
sidebar_position: 3
---

import Asciinema from '../src/asciinema.jsx';

# Usage

Klunok uses the Linux kernel fanotify API, which as of now requires root privileges.
Therefore, Klunok should be invoked, for example, with `sudo`:

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
You can use
[the `-d` command line option](./cli.md#-d-path-to-a-file-or-directory-which-owners-identity-will-be-used-for-running-klunok)
to customize this.

Once Klunok successfully starts,
it watches for file edits in the current working directory and its descendants.
When it thinks that the current version of a file is more or less stable
(the file hasn't been edited for one minute),
it copies the file to the `klunok/store` directory.

Klunok considers only files that are edited by applications that humans use to edit files,
for example, Vim and LibreOffice.
Please note that Klunok must be launched before these applications are to recognize them.

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
