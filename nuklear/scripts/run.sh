#!/usr/bin/env bash
set -euo pipefail
repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cmake -S "$repo_dir" -B "$repo_dir/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$repo_dir/build"
if command -v i3-msg >/dev/null 2>&1 && command -v jq >/dev/null 2>&1; then
  left_workspace=$(i3-msg -t get_workspaces | jq -r 'map(select(.visible)) | sort_by(.rect.x) | .[0].name')
  (
    for _ in $(seq 1 100); do
      if i3-msg "[title=\"^Gubsy Nuklear trial$\"] move container to workspace \"$left_workspace\", move position center" 2>/dev/null | jq -e '.[0].success' >/dev/null; then
        exit 0
      fi
      sleep 0.02
    done
  ) &
fi
exec "$repo_dir/build/gubsy-nuklear-trial" "$@"
