{ stdenv
, meson
, ninja
, pkg-config
, lua
, valgrind
}: stdenv.mkDerivation {
  pname = "klunok";
  version = "0.1.0-" + (if lua == null then "no-lua" else "lua-${lua.version}");
  src = ./.;
  doCheck = true;

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];
  buildInputs = [
    lua
  ];
  checkInputs = [
    valgrind
  ];
}
