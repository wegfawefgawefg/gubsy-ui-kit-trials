# Arbor trial

This trial recreates the shared Gubsy shell with Arbor's retained AXL runtime,
Yoga layout, SDL3 input, and SDL_GPU renderer. It is a direct capability probe,
not an Arbor/Gubsy integration layer.

## Run

```sh
./scripts/run.sh
./scripts/run.sh --screen mods
./scripts/run.sh --resolution 1920x1080
```

The launcher builds a release binary and asks i3 to float it at 1280x720 on the
leftmost visible workspace. The executable itself accepts `play`, `players`,
`settings`, `controls`, `progress`, or `mods` for `--screen`.

The host composes SDL's output-DPI scale with a 720p game-space scale. A
1920x1080 window therefore renders the same usable composition at 1.5x rather
than shrinking the interface into the upper-left of a larger logical canvas.

Mouse and keyboard use Arbor's native widget path. Attached SDL gamepads map
D-pad navigation, A/select, B/back, and shoulders onto the same retained widget
interactions. This is sequential focus traversal; Arbor does not currently
provide a spatial or authored controller focus graph.

## Benchmark

```sh
./build/gubsy-arbor-trial --hidden --benchmark 1200 \
  --resolution 1280x720 --screen mods
```

The JSON report separates Arbor reconcile, style, layout, render-list, and total
retained UI work from the complete host render call. RSS is the whole process,
including SDL, the Vulkan driver, swapchain resources, font state, and all six
resident route documents.

## Capability gaps found

- AXL has no image widget or texture primitive, so the catalog deliberately
  labels the artwork region instead of hiding that gap.
- The supported style subset is materially smaller than browser CSS: no media
  queries, wrapping, per-edge borders, or several common flex constraints.
- One font is loaded and a missing glyph is fatal; there is no fallback stack.
- Tabs use a value-bearing button contract and generated model event handlers.
  The host must recognize shell route payloads rather than depend on a stable
  generated handler name.
- Gamepad navigation is keyboard-like sequential traversal rather than local
  spatial relationships. It works, but is below the RmlUi trial's authored
  focus-graph behavior.
- B/back is handled by the host. Arbor does not expose whether a native select
  popup is open, so cancel-popup-before-leaving-screen needs a runtime API.
- A responsive phone composition would require separate authored AXL state;
  this trial targets the required 1280x720 and 1920x1080 game layouts.

Those are product findings, not excuses for inactive controls: selects,
checkboxes, sliders, tabs, scrolling, route changes, checkpoint selection,
session rules, and session-mod management use real Arbor interactions.
