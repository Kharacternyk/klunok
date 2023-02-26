{ stdenv
, meson
, ninja
, pkg-config
, lua
, valgrind
, ldoc ? null
, doCheck ? false
}: stdenv.mkDerivation {
  pname = "klunok";
  version = "0.1.0-" + (if lua == null then "no-lua" else "lua-${lua.version}");
  src = ./.;

  inherit doCheck;

  mesonFlags = if ldoc != null then [ "-Dbuild_doc=true" ] else [ ];

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
    ldoc
  ];
  buildInputs = [
    lua
  ];
  checkInputs = [
    valgrind
  ];
}
