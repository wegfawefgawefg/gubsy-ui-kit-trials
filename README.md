# Gubsy UI kit trials

This repository compares native UI approaches against the same Gubsy shell
interaction and content target. Visual theming is intentionally secondary to
layout, control behavior, game embedding, and runtime cost.

All implementations, shared scripts, reference assets, and evaluation results
live in this single repository so changes can be reviewed and benchmarked as a
coherent comparison suite.

## Implementations

- `vue-reference/` — browser design/reference prototype.
- `rmlui/` — retained HTML/CSS-like native implementation on SDL3 + SDL_GPU.
- `arbor/` — retained AXL/Yoga implementation on SDL3 + SDL_GPU.
- `dear-imgui/` — immediate-mode implementation on SDL3 + SDL_GPU.
- `raygui/` — immediate-mode implementation on raylib.
- `nuklear/` — immediate-mode implementation using Nuklear.

Clay is deliberately excluded from this round. A Clay trial would require a
Gubsy-owned interaction/widget layer and belongs with a possible Arbor rewrite.

## Run

Every native implementation accepts a viewport argument and starts as a
centered 1280x720 utility window by default:

```sh
./scripts/run.sh rmlui
./scripts/run.sh arbor
./scripts/run.sh dear-imgui
./scripts/run.sh raygui
./scripts/run.sh nuklear
```

Pass extra arguments after the implementation name, for example:

```sh
./scripts/run.sh raygui --resolution 1920x1080
```

See `evaluation-notes/CONTRACT.md` for the common comparison target and
`evaluation-notes/RESULTS.md` for measured results.
