{
  inputs.utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, utils }: utils.lib.eachDefaultSystem (system:
    with import nixpkgs { inherit system; }; {
      defaultPackage = callPackage ./. {};
      devShell = mkShell {
        CPATH = "${fuse3}/include/fuse3";
      };
    }
  );
}
