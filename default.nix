{
  doCheckThoroughly ? (
    lib.meta.availableOn stdenv.hostPlatform valgrind-light && !valgrind-light.meta.broken
  ),
  lib,
  lua,
  meson,
  musl-fts,
  ninja,
  pkg-config,
  stdenv,
  valgrind-light,
}:
stdenv.mkDerivation {
  buildInputs = [
    lua
  ]
  ++ lib.optionals stdenv.targetPlatform.isMusl [
    musl-fts
  ];
  checkInputs = lib.optionals doCheckThoroughly [
    valgrind-light
  ];
  doCheck = !stdenv.targetPlatform.isStatic;
  mesonFlags = lib.optionals (!stdenv.targetPlatform.isStatic) [
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
