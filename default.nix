{
  stdenv,
  autoreconfHook,
  libtool,
  pkg-config,
}: stdenv.mkDerivation {
  pname = "tealfs";
  version = "0.1.0";
  src = ./.;

  nativeBuildInputs = [
    autoreconfHook
    libtool
    pkg-config
  ];
}
