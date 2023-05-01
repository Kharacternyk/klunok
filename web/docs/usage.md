---
sidebar_position: 3
---

# Usage

Klunok uses the Linux kernel fanotify API, which as of now requires root privileges.
For this reason, Klunok should be invoked, for example, with `sudo`:

```bash
sudo klunok
```

If Klunok is invoked without the necessary privileges, you will face an error:

```
Cannot initialize fanotify
└─┤because of│ Operation not permitted
```

Klunok will drop privileges to the owner of the current working directory
after initializing the fanotify API.
The complete picture of security measures and implications is described in
[the security section](security).
