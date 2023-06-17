# Security

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

The output of `cmp` should be empty.
