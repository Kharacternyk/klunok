{
  callPackage,
  gdb,
  lib,
  libseccomp,
  lua,
  meson,
  mkShell,
  ninja,
  pkg-config,
  stdenv,
}:
stdenv.mkDerivation (finalAttrs: {
  buildInputs = [
    libseccomp
    lua
  ];
  doCheck = true;
  env.NIX_CFLAGS_COMPILE = "-fsanitize=signed-integer-overflow -fsanitize-trap=signed-integer-overflow";
  hardeningDisable = [
    "strictoverflow"
  ];
  hardeningEnable = [
    "trivialautovarinit"
  ];
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
