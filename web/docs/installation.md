---
sidebar_position: 2
---

import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

# Installation

You can choose from three installation methods.
Please navigate to the one you prefer via the tabs below.

<Tabs>
  <TabItem value="binary" label="Static binary">

You can download a self-contained binary from
<https://github.com/Kharacternyk/klunok/releases/latest/download/klunok>.
You can also browse past releases at
<https://github.com/Kharacternyk/klunok/releases>.

Once downloaded, mark the file as executable:

```bash
chmod +x ~/Downloads/klunok
```

Also, you can copy the binary to somewhere in you `$PATH`, for example:

```bash
sudo cp ~/Downloads/klunok /bin/
```

If you skip this step,
do not forget to specify the full path to the binary when invoking Klunok, for example
`~/Downloads/klunok`.

The binaries should work on all x86-64 Linux systems.
They will not work on architectures other than x86-64,
for example ARM on Raspberry Pi or 32-bit x86.
Please use alternative installation methods for other architectures.

  </TabItem>

  <TabItem value="nix" label="Nix flake">

You can install Klunok with a flake-enabled version of the Nix package manager:

```bash
nix profile install github:Kharacternyk/klunok/v1
```

Avoid installing `github:Kharacternyk/klunok` (without a version tag),
as this is the development version that is more likely to have bugs.
Instead use `github:Kharacternyk/klunok/v1` for the latest stable version.

  </TabItem>
  <TabItem value="source" label="From source">

There are no detailed instructions for this one, yet.
If you are familiar with C and the Meson build system,
we're sure you know what to do once you get the source from
<https://github.com/Kharacternyk/klunok>.

  </TabItem>
</Tabs>
