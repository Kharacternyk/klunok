---
description: Klunok is a smart versioning and automatic backup daemon for Linux.
sidebar_position: 1
sidebar_label: Introduction
---

import Logo from '../static/logo.svg';

# <Logo alt="" height="100%" width="2em" style={{verticalAlign: "middle"}} /> Klunok

<head>
  <title>Klunok</title>
</head>

Klunok is a smart versioning and automatic backup daemon for Linux.
It keeps a versioned history of files that you edit,
doing so in the background without any effort required from you.
It picks up everything that matters (sources, …) and nothing that doesn't (binaries, …)
automatically.

Klunok works well with most text and graphics editors, IDEs, and office suites,
including Vim, Visual Studio Code, LibreOffice, and Inkscape.
Also, Klunok is free and open source: https://github.com/Kharacternyk/klunok.

```mermaid
graph TD;
  subgraph "Work as you usually do…"
  w[Edit your files.];
  s[Hit save.];
  end
  q[The files are scheduled for backup.];
  b[The files are backed up.];
  w --> s;
  s --> w;
  s -.-> |"…and Klunok will take care of the rest."| q;
  q -.-> |"A minute passes with no further edits."| b;
```
