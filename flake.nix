{
  outputs = { self, nixpkgs, flake-utils }:
    let
      utils = flake-utils.lib;
      system = utils.system;
      supportedSystems = [ system.aarch64-linux system.x86_64-linux system.i686-linux ];
    in
    utils.eachSystem supportedSystems (system:
      let
        pkgs = import nixpkgs { inherit system; };
        checks =
          let
            mkCheck = { callPackage, lua, valgrind }: callPackage ./. {
              inherit lua valgrind;
              doCheck = true;
            };
            glibcChecks = builtins.mapAttrs
              (_: lua: mkCheck {
                inherit (pkgs) callPackage valgrind;
                inherit lua;
              })
              {
                inherit (pkgs) lua5_4 lua5_3 lua5_2;
                withoutLua = null;
              };
            muslChecks = if system == utils.system.i686-linux then { } else {
              muslWithoutLua = mkCheck {
                inherit (pkgs.pkgsMusl) callPackage;
                lua = null;
                valgrind = null;
              };
            };
          in
          glibcChecks // muslChecks;
      in
      {
        inherit checks;
        packages =
          let mkPackage = pkgs': pkgs'.callPackage ./. { lua = pkgs'.lua5_4; }; in
          {
            default = mkPackage pkgs;
            static = mkPackage pkgs.pkgsStatic;
          };
        devShells = builtins.mapAttrs
          (_: package: pkgs.mkShell {
            inputsFrom = [
              package
            ];
          })
          checks;
      }
    );
}
