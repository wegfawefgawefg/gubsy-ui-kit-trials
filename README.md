# Gubsy RmlUi Design Demo

Native FOSS feasibility experiment for reproducing the approved
[`gubsy-ui-design-demo`](../gubsy-ui-design-demo) interaction and layout in a
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
```

Press `F1` to toggle the ImGui experiment panel and `Escape` to exit. The panel
switches target screens, applies the acceptance viewport presets, changes the
RmlUi density-independent-pixel ratio, and displays initial CPU/RSS telemetry.

The checked-in RmlUi document is only a native host/proportions smoke surface.
It is not presented as the completed Vue replica; that is the work described in
[the port plan](docs/PORT_PLAN.md).

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
