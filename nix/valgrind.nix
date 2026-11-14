{
  lib,
  stdenv,
  valgrind-light,
}:
if
  lib.meta.availableOn stdenv.hostPlatform valgrind-light && !valgrind-light.meta.broken
then
  valgrind-light
else
  null
