#!/usr/bin/env bash
# Verify an iOS static engine library is a real library-mode build for the
# expected platform, so a wrong build fails CI instead of shipping.
#
#   check_ios_library.sh <lib.a> <platform>
#
# <platform> is the Mach-O LC_BUILD_VERSION platform: 2 = iOS device,
# 7 = iOS Simulator.
#
# nm/otool output is buffered into variables before grepping: `nm | grep -q`
# stops reading at the first match, nm dies of SIGPIPE, and under pipefail the
# check misreads success as failure.
set -euo pipefail

lib="$1"
platform="$2"

if [[ ! -f "$lib" ]]; then
  echo "::error::$lib not found; bin/ has:"
  ls -l bin/ || true
  exit 1
fi

syms=$(nm -arch arm64 "$lib" 2>/dev/null || true)
if ! grep -Eq '^[0-9a-f]+ T _godot_library_start$' <<<"$syms"; then
  echo "::error::$lib does not export _godot_library_start (ios_library=yes not applied)"
  exit 1
fi
if grep -Eq '^[0-9a-f]+ T _main$' <<<"$syms"; then
  echo "::error::$lib still exports _main (IOS_LIBRARY_MODE not defined)"
  exit 1
fi

load_cmds=$(otool -l "$lib")
platforms=$(awk '/cmd LC_BUILD_VERSION/{f=1} f && /platform/{print $2; f=0}' <<<"$load_cmds" | sort -u)
if [[ "$platforms" != "$platform" ]]; then
  echo "::error::$lib targets platform(s) '$(tr '\n' ' ' <<<"$platforms")', expected $platform"
  exit 1
fi

echo "$lib: library-mode, platform $platform ✓"
