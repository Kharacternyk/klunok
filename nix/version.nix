lua:
let
  baseVersion = builtins.readFile ../version;
  luaSuffix = if lua == null then "no-lua" else "lua-${lua.version}";
in
"${baseVersion}-${luaSuffix}"
