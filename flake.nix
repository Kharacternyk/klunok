{
  outputs = { self, nixpkgs, flake-utils }: flake-utils.lib.eachDefaultSystem (system:
    with import nixpkgs { inherit system; };
    let
      klunok = callPackage ./. { };
    in
    {
      packages.default = klunok;
      devShells.default = mkShell {
        inputsFrom = [
          klunok
        ];
        packages = [
          gcovr
          valgrind
          gdb
        ];
      };
    }
  );
}
