{
  stdenv,
  meson,
  ninja,
  pkg-config,
  fuse3
}: stdenv.mkDerivation {
  pname = "dimfs";
  version = "0.1.0";
  src = ./.;

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];
  buildInputs = [
    fuse3
  ];
}
