#!/usr/bin/env bash
set -euo pipefail

workspace_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
output_dir=${1:-"$workspace_dir/evaluation-notes/captures"}
screens=(play players settings controls progress mods)
viewports=(1280x720 1920x1080 915x412 412x915)

for trial in dear-imgui raygui nuklear; do
  cmake -S "$workspace_dir/$trial" -B "$workspace_dir/$trial/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build "$workspace_dir/$trial/build"
done

for viewport in "${viewports[@]}"; do
  mkdir -p "$output_dir/$viewport"
  for screen in "${screens[@]}"; do
    "$workspace_dir/dear-imgui/build/gubsy-dear-imgui-trial" \
      --resolution "$viewport" --screen "$screen" \
      --capture "$output_dir/$viewport/dear-imgui-$screen.bmp"
    "$workspace_dir/raygui/build/gubsy-raygui-trial" \
      --resolution "$viewport" --screen "$screen" \
      --capture "$output_dir/$viewport/raygui-$screen.png"
    "$workspace_dir/nuklear/build/gubsy-nuklear-trial" \
      --resolution "$viewport" --screen "$screen" \
      --capture "$output_dir/$viewport/nuklear-$screen.bmp"
  done
done

printf 'Captured %d backend/route/viewport frames in %s\n' "$((3 * 6 * 4))" "$output_dir"
