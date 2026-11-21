{
  lib,
  pkgs,
  self,
  ...
}:
rec {
  checks = lib.mapAttrs (_: package: package.check) packages;
  devShells = lib.mapAttrs (_: package: package.devShell) packages;
  packages = lib.packagesFromDirectoryRecursive {
    callPackage = lib.callPackageWith (
      pkgs
      // {
        inherit self;
        klunok = packages.default;
        lua = pkgs.lua5_5;
      }
    );
    directory = ./packages;
  };
}
