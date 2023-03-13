{
  outputs = { self, nixpkgs, flake-utils }:
    let
      utils = flake-utils.lib;
      system = utils.system;
      supportedSystems = [ system.aarch64-linux system.x86_64-linux system.i686-linux ];
    in
    utils.eachSystem supportedSystems (system:
      let
        getLuaPackages = pkgs: {
          inherit (pkgs) lua5_4 lua5_3 lua5_2;
          default = pkgs.lua5_4;
          withoutLua = null;
        };
        inherit (builtins) mapAttrs;
        getPackages = pkgs: mapAttrs
          (_: lua: pkgs.callPackage ./. { inherit lua; })
          (getLuaPackages pkgs);
        pkgs = import nixpkgs { inherit system; };
        prefixAttrs = prefix: pkgs.lib.mapAttrs' (name: value: {
          inherit value;
          name = prefix + name;
        });
        packages = getPackages pkgs // (
          prefixAttrs "static-" (getPackages pkgs.pkgsStatic)
        );
        checks = mapAttrs (_: package: package.override { doCheck = true; }) packages;
      in
      {
        inherit checks packages;
        devShells = mapAttrs
          (_: package: pkgs.mkShell {
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
