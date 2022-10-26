{
  outputs = { self, nixpkgs, flake-utils }: flake-utils.lib.eachDefaultSystem (system:
    with import nixpkgs { inherit system; };
    let
      tealfs = callPackage ./. { };
    in
    {
      defaultPackage = tealfs;
      devShell = mkShell {
        inputsFrom = [
          tealfs
        ];
        CPATH = "${fuse3}/include/fuse3";
      };
    }
  );
}
