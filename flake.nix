{
  outputs = { self, nixpkgs, flake-utils }:
    let
      utils = flake-utils.lib;
      system = utils.system;
      supportedSystems = [ system.aarch64-linux system.x86_64-linux system.i686-linux ];
    in
    utils.eachSystem supportedSystems (system:
      let
        inherit (builtins) mapAttrs;
        pkgs = import nixpkgs { inherit system; };

        packages =
          let mkPackage = pkgs': pkgs'.callPackage ./. { lua = pkgs'.lua5_4; }; in
          {
            default = mkPackage pkgs;
            static = mkPackage pkgs.pkgsStatic;
          };

        checks =
          let mkCheck = { callPackage, lua, valgrind-light }: callPackage ./. {
            inherit lua valgrind-light;
            doCheck = true;
          }; in
          {
            glibc = mapAttrs
              (_: lua: mkCheck {
                inherit (pkgs) callPackage valgrind-light;
                inherit lua;
              })
              {
                inherit (pkgs) lua5_4 lua5_3 lua5_2;
                withoutLua = null;
              };
            musl = if system == utils.system.i686-linux then { } else {
              muslWithoutLua = mkCheck {
                inherit (pkgs.pkgsMusl) callPackage;
                lua = null;
                valgrind-light = null;
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
                };
              in
              {
                glibc = mapAttrs (_: mkShell pkgs) checks.glibc;
                musl = mapAttrs (_: mkShell pkgs.pkgsMusl) checks.musl;
                default = {
                  default = mkShell pkgs (packages.default.override {
                    doCheck = true;
                  });
                  static = mkShell pkgs.pkgsStatic packages.static;
                };
              };
          in
          devShells.glibc // devShells.musl // devShells.default;
      }
    );
}
