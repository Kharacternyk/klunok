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
  env.NIX_CFLAGS_COMPILE = ''
    -fsanitize=signed-integer-overflow
    -fsanitize-trap=signed-integer-overflow
    -ftrivial-auto-var-init=zero
  '';
  hardeningDisable = [
    "strictoverflow"
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
    check = finalAttrs.finalPackage.overrideAttrs (previous: {
      checkInputs = [
        (callPackage ../valgrind.nix {
          inherit stdenv;
        })
      ];
      env = previous.env // {
        NIX_CFLAGS_COMPILE = ''
          ${previous.env.NIX_CFLAGS_COMPILE}
          -ftrivial-auto-var-init=uninitialized
        '';
      };
    });
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
