{
  outputs = { self, nixpkgs, flake-utils }: flake-utils.lib.eachDefaultSystem (system:
    with import nixpkgs { inherit system; };
    let
      klunok = callPackage ./. { };
    in
    {
      defaultPackage = klunok;
      devShell = mkShell {
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
