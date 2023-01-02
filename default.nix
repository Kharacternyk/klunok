{ stdenv
, meson
, ninja
, pkg-config
, lua
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
    lua
  ];
}
