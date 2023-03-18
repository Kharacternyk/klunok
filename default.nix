{ stdenv
, meson
, ninja
, pkg-config
, lua
, valgrind-light
, doCheck ? false
}: stdenv.mkDerivation {
  pname = "klunok";
  version = builtins.readFile ./version + (if lua == null then "no-lua" else "lua-${lua.version}");
  src = ./.;

  inherit doCheck;

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];
  buildInputs = [
    lua
  ];
  checkInputs = [
    valgrind-light
  ];
}
