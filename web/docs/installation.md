---
sidebar_position: 2
---

import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

# Installation

You can choose from three installation methods.
Please navigate to the one you prefer via the tabs below.

<Tabs queryString="method">
  <TabItem value="binary" label="Static binary">

You can download a self-contained binary from
https://github.com/Kharacternyk/klunok/releases/latest/download/klunok.
You can also browse past releases at https://github.com/Kharacternyk/klunok/releases.

Once the file is downloaded, mark it as executable:

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

[The binaries are reproducible.](./security.md#static-binary-reproducibility)

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

Installing from source requires a C compiler and the Meson build system.
Meson also depends on the Ninja build system.
You can install them, for example, with `apt`:

```bash
sudo apt install gcc meson ninja-build
```

The version of Meson installed with `apt` may not be new enough to build Klunok.
In this case, you can install Meson with `pip`:

```bash
sudo apt install pip
sudo pip install meson
```

Lua is an optional but recommended dependency.
It allows configuring Klunok without recompiling it.
Klunok should work with Lua version 5.2 or newer.
Installing Lua with `apt` looks like this:

```bash
sudo apt install lua5.4
```

Once the dependencies are installed, get the source from
[GitHub](https://github.com/Kharacternyk/klunok) and install Klunok with Meson.
The whole process may look like this:

```bash
git clone https://github.com/Kharacternyk/klunok
cd klunok
git checkout v1
meson setup build -Dbuildtype=release
cd build
sudo meson install
```

  </TabItem>
</Tabs>
