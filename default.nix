{
  doCheckThoroughly ? (
    lib.meta.availableOn stdenv.hostPlatform valgrind-light && !valgrind-light.meta.broken
  ),
  lib,
  lua,
  meson,
  ninja,
  pkg-config,
  static,
  stdenv,
  valgrind-light,
}:
stdenv.mkDerivation {
  buildInputs = [
    (lua.override {
      staticOnly = static;
    })
  ]
  ++ lib.optionals static [
    stdenv.cc.libc.static
  ];
  checkInputs = lib.optionals doCheckThoroughly [
    valgrind-light
  ];
  doCheck = !static;
  env = lib.optionalAttrs static {
    NIX_CFLAGS_LINK = "-static";
  };
  mesonFlags = lib.optionals (!static) [
    "-Dwatch_nix_store=true"
  ];
  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];
  pname = "klunok";
  src = ./.;
  version =
    builtins.readFile ./version + (if lua == null then "no-lua" else "lua-${lua.version}");
}
