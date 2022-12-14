{ stdenv
, meson
, ninja
, pkg-config
, lua5_4
}: stdenv.mkDerivation {
  pname = "klunok";
  version = "0.1.0";
  src = ./.;

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];
  buildInputs = [
    lua5_4
  ];
}
