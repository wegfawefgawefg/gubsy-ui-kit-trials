#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
output=${1:?usage: capture.sh OUTPUT [SCREEN] [RESOLUTION]}
screen=${2:-play}
resolution=${3:-1280x720}

if [[ ! -x "$repo_dir/build/gubsy-arbor-trial" ]]; then
  cmake -S "$repo_dir" -B "$repo_dir/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build "$repo_dir/build"
fi

xvfb-run -a -s "-screen 0 ${resolution}x24 -nolisten tcp" bash -c '
  set -euo pipefail
  binary=$1
  output=$2
  screen=$3
  resolution=$4
  "$binary" --screen "$screen" --resolution "$resolution" &
  app_pid=$!
  trap "kill $app_pid 2>/dev/null || true" EXIT
  for _ in $(seq 1 100); do
    window=$(xdotool search --name "^Gubsy Arbor trial$" 2>/dev/null | head -1 || true)
    if [[ -n "$window" ]]; then
      sleep 0.15
      import -window "$window" "$output"
      exit 0
    fi
    sleep 0.05
  done
  printf "Arbor capture window did not appear\n" >&2
  exit 1
' capture "$repo_dir/build/gubsy-arbor-trial" "$output" "$screen" "$resolution"
