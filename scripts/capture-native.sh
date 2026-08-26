#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
output_dir=${1:-"$repo_dir/build/native-captures"}

cmake -S "$repo_dir" -B "$repo_dir/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$repo_dir/build"

screens=(
  play-lobby play-quest play-settings play-session-mods
  players players-profiles players-devices
  settings settings-audio settings-accessibility settings-gameplay
  controls-bindings controls-devices controls-input-tuning
  progress mods-installed mods-catalog
)
viewports=(1920x1080 1280x720 412x915 915x412)

for viewport in "${viewports[@]}"; do
  mkdir -p "$output_dir/$viewport"
  for index in "${!screens[@]}"; do
    "$repo_dir/build/gubsy-rmlui-demo" \
      --resolution "$viewport" \
      --screen "$index" \
      --capture "$output_dir/$viewport/${screens[$index]}.png"
  done
done

printf 'Captured %d native frames in %s\n' "$(( ${#screens[@]} * ${#viewports[@]} ))" "$output_dir"
