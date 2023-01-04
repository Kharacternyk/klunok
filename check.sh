#!/bin/sh

cd "${MESON_BUILD_ROOT}"
meson test --wrap='valgrind --error-exitcode=1 --leak-check=full' --print-errorlogs "$@"
