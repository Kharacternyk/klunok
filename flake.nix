{
  description = "A smart versioning and automatic backup daemon";
  outputs =
    {
      flake-utils,
      nixpkgs,
      self,
    }:
    let
      inherit (flake-utils.lib) system;
      supportedSystems = [
        system.aarch64-linux
        system.x86_64-linux
        system.i686-linux
        system.armv7l-linux
      ];
    in
    flake-utils.lib.eachSystem supportedSystems (
      system:
      let
        inherit (builtins) mapAttrs;
        checks =
          let
            mkCheck =
              {
                callPackage,
                lua,
              }:
              callPackage ./. {
                inherit lua;
              };
          in
          mapAttrs
            (
              _: lua:
              mkCheck {
                inherit (pkgs) callPackage;
                inherit lua;
              }
            )
            {
              inherit (pkgs) lua5_2 lua5_3 lua5_4;
              withoutLua = null;
            };
        packages =
          let
            mkPackage =
              pkgs':
              pkgs'.callPackage ./. {
                doCheckThoroughly = false;
                lua = pkgs'.lua5_4;
              };
          in
          {
            default = mkPackage pkgs;
            static = mkPackage pkgs.pkgsStatic;
          };
        pkgs = import nixpkgs {
          inherit system;
        };
      in
      {
        inherit checks packages;
        devShells =
          let
            devShells =
              let
                mkShell =
                  pkgs': package:
                  pkgs'.mkShell {
                    inputsFrom = [
                      package
                    ];
                    packages = [
                      pkgs.gdb
                    ];
                  };
              in
              {
                default = {
                  default = mkShell pkgs (
                    packages.default.override {
                      doCheckThoroughly = true;
                    }
                  );
                  static = mkShell pkgs.pkgsStatic packages.static;
                };
                glibc = mapAttrs (_: mkShell pkgs) checks.glibc;
              };
          in
          devShells.glibc // devShells.default;
      }
    );
}
