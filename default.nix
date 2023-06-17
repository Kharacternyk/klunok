{ lib
, stdenv
, meson
, ninja
, pkg-config
, lua
, valgrind-light
, musl-fts
, doCheck ? false
}: stdenv.mkDerivation {
  pname = "klunok";
  version = builtins.readFile ./version
    + (if lua == null then "no-lua" else "lua-${lua.version}");
  src = ./.;

  inherit doCheck;

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
  checkInputs = [
    valgrind-light
  ];
}
