---
sidebar_position: 1
---

# Introduction

Klunok is a smart versioning and automatic backup daemon for Linux.
It keeps a versioned history of files that you edit,
doing so in the background without any effort required from you.

Klunok works well with most text and graphics editors, IDEs, and office suites,
including Vim, Visual Studio Code, LibreOffice, and Inkscape.

```mermaid
graph TD;
  subgraph "Work as you usually do…"
  w[Edit your files];
  s[Hit save];
  end
  q[The files are scheduled for backup];
  b[The files are backed up];
  w -->  s;
  s --> w;
  s -.-> |"…and Klunok will take care of the rest"| q;
  q -.-> |"…a minute (by default) passes with no further edits…"| b;
```
