---
sidebar_position: 7
---

# Editors

If you want to let Klunok back up files edited with an application that is not recognized
as an editor by default,
add the name of the executable that is normally used to start the application
to [the `editors` setting](./configuration.md#editors).
For example, if you type `awesomeeditor file.txt` in the terminal
when you want to edit `file.txt` with `awesomeeditor`, add `editors.awesomeeditor = true`
to the configuration file.

This works for many applications on most Linux distributions.
If it doesn't work,
some investigation is needed to find out what executable
writes to the file on behalf of the application.
Please [open a GitHub issue](https://github.com/Kharacternyk/klunok/issues/new/choose)
or ping nazar@vinnich.uk and we'll look into it.

Unfortunately, some applications are not fully compatible with Klunok.
These include:

- some interpreted applications, for example Python scripts;
- applications that use `mmap` to edit files;
- applications that write to a temporary file and then replace the edited file with `rename`.
