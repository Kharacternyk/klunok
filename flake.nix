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
        klunokWithLua = lua: pkgs.callPackage ./. { inherit lua; };
        supportedKlunokBuilds = builtins.mapAttrs (name: value: klunokWithLua value) {
          inherit (pkgs) lua5_4 lua5_3 lua5_2;
          default = pkgs.lua5_4;
          withoutLua = null;
        };
      in
      {
        packages = supportedKlunokBuilds;
        checks = supportedKlunokBuilds;
        devShells = builtins.mapAttrs
          (name: value: pkgs.mkShell {
            inputsFrom = [
              value
            ];
            packages = [
              pkgs.gdb
            ];
          })
          supportedKlunokBuilds;
      }
    );
}
