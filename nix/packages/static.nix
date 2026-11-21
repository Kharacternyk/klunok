{
  callPackage,
  klunok,
  lib,
  self,
  stdenv,
}:
(klunok.override (original: {
  libseccomp = original.libseccomp.overrideAttrs (previous: {
    configureFlags = previous.configureFlags or [ ] ++ [
      "--disable-shared"
    ];
    dontDisableStatic = true;
  });
  lua = lib.mapNullable (
    lua:
    lua.override {
      staticOnly = true;
    }
  ) original.lua;
})).overrideAttrs
  (previous: rec {
    buildInputs = previous.buildInputs ++ [
      stdenv.cc.libc.static
    ];
    doCheck = false;
    env = previous.env or { } // {
      NIX_CFLAGS_LINK = "-static";
    };
    mesonFlags = lib.remove "-Dwatch_nix_store=true" previous.mesonFlags ++ [
      "-Ddependency_notices=${passthru.releaseSources.notices}"
    ];
    passthru = previous.passthru // {
      releaseSources = callPackage ../release.nix {
        inherit self;
        inherit (previous) version;
        dependencies = previous.buildInputs ++ [
          stdenv.cc.libc
        ];
      };
    };
  })
