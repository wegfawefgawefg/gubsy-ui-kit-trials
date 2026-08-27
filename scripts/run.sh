#!/usr/bin/env bash
set -euo pipefail
repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cmake -S "$repo_dir" -B "$repo_dir/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$repo_dir/build"
if command -v i3-msg >/dev/null 2>&1; then
  i3-msg 'for_window [title="^Gubsy raygui trial$"] floating enable, move position center' >/dev/null
fi
exec "$repo_dir/build/gubsy-raygui-trial" "$@"
