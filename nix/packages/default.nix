{
  callPackage,
  gdb,
  lib,
  lua,
  meson,
  mkShell,
  ninja,
  pkg-config,
  stdenv,
}:
stdenv.mkDerivation (finalAttrs: {
  buildInputs = [
    lua
  ];
  doCheck = true;
  mesonFlags = [
    "-Dwatch_nix_store=true"
  ];
  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];
  passthru = rec {
    check = finalAttrs.finalPackage.overrideAttrs {
      checkInputs = [
        (callPackage ../valgrind.nix {
          inherit stdenv;
        })
      ];
    };
    devShell = mkShell {
      inputsFrom = [
        check
      ];
      packages = [
        gdb
      ];
    };
  };
  pname = "klunok";
  src = ../..;
  version = lib.fileContents ../../version;
})
