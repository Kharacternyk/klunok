{ lib
, stdenv
, meson
, ninja
, pkg-config
, lua
, valgrind-light
, musl-fts
, doCheckThoroughly ? (
    lib.availableOn stdenv.hostPlatform valgrind-light && !valgrind-light.meta.broken
  )
}: stdenv.mkDerivation {
  pname = "klunok";
  version = builtins.readFile ./version
    + (if lua == null then "no-lua" else "lua-${lua.version}");
  src = ./.;

  doCheck = !stdenv.targetPlatform.isStatic;
  mesonFlags = lib.optionals (!stdenv.targetPlatform.isStatic) [
    "-Dwatch_nix_store=true"
  ];

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];
  buildInputs = [
    lua
  ] ++ lib.optionals stdenv.targetPlatform.isMusl [
    musl-fts
  ];
  checkInputs = lib.optionals doCheckThoroughly [
    valgrind-light
  ];
}
