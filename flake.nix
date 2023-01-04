{
  outputs = { self, nixpkgs, flake-utils }: flake-utils.lib.eachDefaultSystem (system:
    with import nixpkgs { inherit system; };
    let
      klunok = callPackage ./. { };
      check = writeShellScriptBin "check" ''
        meson test --wrap='valgrind --error-exitcode=1 --leak-check=full' --print-errorlogs \
        "$@"
      '';
    in
    {
      defaultPackage = klunok;
      devShell = mkShell {
        inputsFrom = [
          klunok
        ];
        packages = [
          check
          gcovr
          valgrind
          gdb
        ];
      };
    }
  );
}
