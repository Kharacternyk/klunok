{
  klunok,
}:
klunok.overrideAttrs (previous: {
  env = previous.env // {
    NIX_CFLAGS_COMPILE = ''
      ${previous.env.NIX_CFLAGS_COMPILE}
      -fno-sanitize-trap=signed-integer-overflow
    '';
  };
  mesonCheckFlags = [
    "--setup=fast"
  ];
  mesonFlags = previous.mesonFlags ++ [
    "-Db_sanitize=address,undefined"
  ];
})
