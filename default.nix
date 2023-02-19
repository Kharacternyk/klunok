{ stdenv
, meson
, ninja
, pkg-config
, lua5_4
, valgrind
}: stdenv.mkDerivation {
  pname = "klunok";
  version = "0.1.0";
  src = ./.;
  doCheck = true;

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];
  buildInputs = [
    lua5_4
  ];
  checkInputs = [
    valgrind
  ];
}
