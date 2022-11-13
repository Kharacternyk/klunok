{
  stdenv,
  meson,
  ninja,
}: stdenv.mkDerivation {
  pname = "klunok";
  version = "0.1.0";
  src = ./.;

  nativeBuildInputs = [
    meson
    ninja
  ];
}
