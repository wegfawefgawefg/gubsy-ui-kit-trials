#!/usr/bin/env bash
set -euo pipefail

workspace_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
output=${1:-"$workspace_dir/evaluation-notes/benchmark-raw.txt"}
: > "$output"

for viewport in 1280x720 1920x1080; do
  printf 'viewport=%s\n' "$viewport" | tee -a "$output"
  "$workspace_dir/rmlui/build/gubsy-rmlui-demo" --benchmark 1000 \
    --resolution "$viewport" --screen 0 | tee -a "$output"
  "$workspace_dir/dear-imgui/build/gubsy-dear-imgui-trial" --benchmark 1000 \
    --resolution "$viewport" --screen play | tee -a "$output"
  "$workspace_dir/raygui/build/gubsy-raygui-trial" --benchmark 1000 \
    --resolution "$viewport" --screen play | tee -a "$output"
  "$workspace_dir/nuklear/build/gubsy-nuklear-trial" --benchmark 1000 \
    --resolution "$viewport" --screen play | tee -a "$output"
done

for trial in rmlui dear-imgui raygui nuklear; do
  binary=$(find "$workspace_dir/$trial/build" -maxdepth 1 -type f -perm -111 -name 'gubsy-*' -print -quit)
  if [[ -n "$binary" ]]; then
    printf 'binary=%s bytes=%s\n' "$trial" "$(stat -c %s "$binary")" | tee -a "$output"
  fi
done
