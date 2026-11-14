{
  nixpkgs,
  ...
}:
let
  inherit (nixpkgs) lib;
  systems = lib.intersectLists lib.platforms.linux lib.systems.flakeExposed;
  perSystem = lib.genAttrs systems (
    system:
    import ./outputs.nix {
      inherit lib;
      pkgs = nixpkgs.legacyPackages.${system};
    }
  );
in
lib.genAttrs [
  "packages"
  "checks"
  "devShells"
] (name: lib.mapAttrs (_: outputs: outputs.${name}) perSystem)
