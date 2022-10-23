{
  stdenv,
  autoreconfHook,
  libtool,
  pkg-config,
  fuse3
}: stdenv.mkDerivation {
  pname = "tealfs";
  version = "0.1.0";
  src = ./.;

  nativeBuildInputs = [
    autoreconfHook
    libtool
    pkg-config
  ];
  buildInputs = [
    fuse3
  ];
}
