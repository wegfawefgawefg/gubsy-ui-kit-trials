# Gubsy RmlUi Design Demo

Native FOSS feasibility experiment for reproducing the approved
[`vue-reference`](../vue-reference) interaction and layout in a
C++ game process.

This repository deliberately starts as a direct SDL integration, not a general
renderer-neutral Gubsy package. The immediate question is whether a manually
authored RmlUi version can preserve the browser prototype's behavior and sane
responsive layout while meeting game-loop performance targets.

## Stack

- SDL3 window, events, gamepads, swapchain, and GPU command submission.
- RmlUi 6.2 for retained documents, RCSS layout, controls, and data binding.
- Dear ImGui 1.92.4 for experiment controls and instrumentation only.
- RmlUi and ImGui share the same SDL window, GPU device, swapchain, and command
  buffer. No browser runtime or off-screen web texture is involved.

All libraries are FOSS. RmlUi and Dear ImGui are pinned to exact commits. The
host SDL3 is intentionally used so this experiment exercises the same SDL layer
as Gubsy/Splonks; SDL3 3.2 or newer is required through `pkg-config`.
Lato is the primary UI face and DejaVu Sans is bundled as the fallback for
controller/navigation glyphs; their license files live beside the font assets.

## Build and run

```bash
./scripts/run.sh
```

The first configure downloads pinned RmlUi, Dear ImGui, and SDL3_image sources.
Useful arguments:

```bash
./scripts/run.sh --resolution 1920x1080
./scripts/run.sh --resolution 1280x720
./scripts/run.sh --hidden --frames 10
./scripts/run.sh --screen 16
./scripts/run.sh --resolution 1280x720 --screen 16 --capture /tmp/catalog.png
./scripts/run.sh --benchmark 1200 --resolution 1280x720 --screen 16
```

Press `F1` to toggle the ImGui experiment panel and `Escape` to exit. The panel
switches all seventeen committed target views, applies the acceptance viewport
presets and provider states, changes the RmlUi density-independent-pixel ratio,
and displays CPU/GPU/RSS telemetry. The SDL window is created as a utility
window, so i3 floats it immediately and centers it on the primary display.

The native implementation includes the full route matrix, fake local state,
contained list/detail scrolling, geometric keyboard/controller focus, live
gamepad discovery, explicit and listening-based bindings, provider states,
toasts, and guarded modals. Every presented toggle, slider, and dropdown is a
real RmlUi form control backed by local demo state. D-pad or left stick moves
focus, A opens/accepts controls, B backs out, LB/RB changes local tabs, and
left/right adjusts focused ranges or closed selections. Catalog artwork is
reused from the Vue target.

No-focus automation is built in:

```bash
./scripts/validate.sh
./scripts/capture-native.sh
```

`capture-native.sh` renders all 68 route/viewport frames into hidden SDL_GPU
textures and downloads PNGs directly. `validate.sh` also checks twelve
responsive empty/loading/error compositions. Neither moves the desktop pointer,
injects OS input, changes i3 workspaces, or displays a test window.

## Vue reference capture

The committed references cover seventeen states at 1920×1080, 1280×720, 412×915,
and 915×412. To regenerate them while the Vue preview is running on port 4173:

```bash
npm install
npm run capture:vue
```

Override `GUBSY_VUE_URL` or `CHROME_PATH` when needed. See
[the reference index](docs/vue-reference/README.md).

## Prepared work

- [Port phases and native boundaries](docs/PORT_PLAN.md)
- [Vue-to-native parity matrix](docs/PARITY_MATRIX.md)
- [Performance and memory protocol](docs/PERFORMANCE_PLAN.md)
- [Captured responsive references](docs/vue-reference/README.md)
- [Measured native results](docs/PERFORMANCE_RESULTS.md)
