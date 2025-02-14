{
  description = "A smart versioning and automatic backup daemon";
  outputs = { self, nixpkgs, flake-utils }:
    let
      utils = flake-utils.lib;
      system = utils.system;
      supportedSystems = [
        system.aarch64-linux
        system.x86_64-linux
        system.i686-linux
        system.armv7l-linux
      ];
    in
    utils.eachSystem supportedSystems (system:
      let
        inherit (builtins) mapAttrs;
        pkgs = import nixpkgs { inherit system; };

        packages =
          let
            mkPackage = pkgs': pkgs'.callPackage ./. {
              doCheckThoroughly = false;
              lua = pkgs'.lua5_4;
            };
          in
          {
            default = mkPackage pkgs;
            static = mkPackage pkgs.pkgsStatic;
          };

        checks =
          let
            mkCheck = { callPackage, lua }: callPackage ./. {
              inherit lua;
            };
          in
          {
            glibc = mapAttrs
              (_: lua: mkCheck {
                inherit (pkgs) callPackage;
                inherit lua;
              })
              {
                inherit (pkgs) lua5_4 lua5_3 lua5_2;
                withoutLua = null;
              };
            musl = pkgs.lib.optionalAttrs pkgs.stdenv.targetPlatform.is64bit {
              muslWithoutLua = mkCheck {
                inherit (pkgs.pkgsMusl) callPackage;
                lua = null;
              };
            };
          };
      in
      {
        inherit packages;
        checks = checks.glibc // checks.musl;
        devShells =
          let
            devShells =
              let
                mkShell = pkgs': package: pkgs'.mkShell {
                  inputsFrom = [
                    package
                  ];
                  packages = [
                    pkgs.gdb
                  ];
                };
              in
              {
                glibc = mapAttrs (_: mkShell pkgs) checks.glibc;
                musl = mapAttrs (_: mkShell pkgs.pkgsMusl) checks.musl;
                default = {
                  default = mkShell pkgs (packages.default.override {
                    doCheckThoroughly = true;
                  });
                  static = mkShell pkgs.pkgsStatic packages.static;
                };
              };
          in
          devShells.glibc // devShells.musl // devShells.default;
      }
    );
}
