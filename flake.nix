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
        supportedLuaPackages = {
          inherit (pkgs) lua5_4 lua5_3 lua5_2;
          default = pkgs.lua5_4;
          withoutLua = null;
        };
        makePackages = args: builtins.mapAttrs
          (_: lua: pkgs.callPackage ./. (args // { inherit lua; }))
          supportedLuaPackages;
        checks = makePackages { doCheck = true; };
      in
      {
        inherit checks;
        packages = makePackages { };
        devShells = builtins.mapAttrs
          (name: package: pkgs.mkShell {
            inputsFrom = [
              package
            ];
            packages = [
              pkgs.gdb
            ];
          })
          checks;
      }
    );
}
