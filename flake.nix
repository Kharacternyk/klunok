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
        klunok = pkgs.callPackage ./. { };
        klunok-lualess = pkgs.callPackage ./. { lua5_4 = null; };
      in
      {
        packages = {
          inherit klunok klunok-lualess;
          default = klunok;
        };
        devShells.default = pkgs.mkShell {
          inputsFrom = [
            klunok
          ];
          packages = [
            pkgs.gdb
          ];
        };
        checks = {
          inherit klunok klunok-lualess;
        };
      }
    );
}
