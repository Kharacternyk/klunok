{
  klunok,
}:
klunok.overrideAttrs (previous: {
  hardeningDisable = (previous.hardeningDisable or [ ]) ++ [
    "strictoverflow"
  ];
  mesonCheckFlags = [
    "--setup=fast"
  ];
  mesonFlags = previous.mesonFlags ++ [
    "-Db_sanitize=address,undefined"
  ];
})
