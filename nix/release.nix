{
  dependencies,
  lib,
  runCommand,
  self,
  version,
}:
runCommand "klunok-${version}-release-sources"
  {
    inherit version;
    nixpkgsRevision = self.inputs.nixpkgs.rev;
    outputs = [
      "out"
      "notices"
    ];
    sources = lib.pipe dependencies [
      (lib.remove null)
      (map (package: package.src))
    ];
  }
  ''
    mkdir -p "$out" "$notices"
    for source in $sources; do
      cp "$source" "$out/$(stripHash "$source")"
      tar -xf "$source"
    done

    noticeFiles=(libseccomp-*/src/api.c glibc-*/csu/version.c)
    if [ -d lua-* ]; then
      noticeFiles=(lua-*/src/lua.h "''${noticeFiles[@]}")
    fi
    gawk -f ${../awk/library-notices.awk} \
      -v release_version="$version" -v nixpkgs_revision="$nixpkgsRevision" \
      "''${noticeFiles[@]}" > "$notices/intro"
    cat glibc-*/LICENSES >> "$notices/intro"
    cp glibc-*/COPYING.LIB "$notices/licenses"
  ''
