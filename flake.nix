{
  outputs = { self, nixpkgs, flake-utils }: flake-utils.lib.eachDefaultSystem (system:
    with import nixpkgs { inherit system; };
    let
      dimfs = callPackage ./. { };
    in
    {
      defaultPackage = dimfs;
      devShell = mkShell {
        inputsFrom = [
          dimfs
        ];
        CPATH = "${fuse3}/include/fuse3";
      };
    }
  );
}
