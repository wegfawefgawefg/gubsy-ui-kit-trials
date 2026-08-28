# GView production-proof results

Measured on 2026-08-28 from the release GView trial. Each run rendered 2,000
hidden uncapped frames after discarding 120 warmup frames. `stable`, `value`,
and `layout` use the Play lobby. `scroll` uses the 20-package catalog and changes
its internal scroll offset every frame.

## CPU frame cost

| Viewport | Scenario | Update | Render recording | Full CPU frame | p99 | Max |
|---|---|---:|---:|---:|---:|---:|
| 1280x720 | Stable | <0.0001 ms | 0.1448 ms | 0.2562 ms | 0.3108 ms | 0.6798 ms |
| 1280x720 | Value dirty | 0.0082 ms | 0.1424 ms | 0.2603 ms | 0.2864 ms | 0.3645 ms |
| 1280x720 | Layout dirty | 0.0092 ms | 0.1408 ms | 0.2612 ms | 0.3126 ms | 0.5851 ms |
| 1280x720 | Dense scroll | 0.0092 ms | 0.1529 ms | 0.2752 ms | 0.3203 ms | 1.1557 ms |
| 1920x1080 | Stable | 0.0001 ms | 0.1425 ms | 0.2551 ms | 0.3230 ms | 1.1280 ms |
| 1920x1080 | Value dirty | 0.0083 ms | 0.1454 ms | 0.2696 ms | 0.3284 ms | 1.9040 ms |
| 1920x1080 | Layout dirty | 0.0093 ms | 0.1431 ms | 0.2666 ms | 0.3450 ms | 1.8944 ms |
| 1920x1080 | Dense scroll | 0.0107 ms | 0.1872 ms | 0.3171 ms | 0.3785 ms | 1.9740 ms |

Compile took 0.020-0.040 ms and runtime activation took 0.006-0.010 ms.
Stable frames produced one layout and one paint build for the full 2,000-frame
run. Value changes rebuilt paint but not layout; forced geometry changes rebuilt
both. The dense 1080p scrolling mean is under one tenth of the 3 ms target and
comfortably below the 6.94 ms 144 Hz frame interval.

These are CPU timings on one development machine, not a hardware guarantee.
The SDL renderer timer covers command recording; the full timer additionally
covers input/update, ImGui's empty release-trial frame, submission, and the
benchmark present path.

## Memory and executable envelope

| Workload | GView-owned estimate | Whole-process peak RSS |
|---|---:|---:|
| 720p Play | 139,138 bytes | 137,072 KiB |
| 720p dense catalog | 286,466 bytes | 136,308 KiB |
| 1080p Play | 139,140 bytes | 136,744 KiB |
| 1080p dense catalog | 287,204 bytes | 136,780 KiB |

The release trial executable is 6,496,408 bytes. It statically includes the
optional Dear ImGui authoring UI and SDL_image in addition to GView, GLayout,
GSEXP, and the full 18-state trial. It is not the size of `gview::core`.
For scale, a separate optimized core-only build produced a 579,456-byte static
archive before final executable linking and linked no SDL, ImGui, or authoring
target. Archive size is not identical to contributed executable text, but it
demonstrates the release boundary directly.

`Runtime::owned_bytes()` estimates the compiled layout/view, control/scroll
state, focus memory, and paint commands. RSS includes SDL, Mesa/Vulkan/OpenGL
userspace, render buffers, font/image textures, allocators, ImGui, and process
startup. The two numbers are intentionally not presented as interchangeable.

## Functional evidence

- 18 complete states cover the full shell and a non-menu inventory.
- FreeType/HarfBuzz text, real images, internal clipping/scrolling, custom native
  surfaces, buttons, toggles, sliders, selects, text input, and overlays render.
- Mouse, keyboard, opened/hot-plugged gamepads, local focus groups, remembered
  entry, explicit Confirm/Back/directional edges, and control capture are wired.
- The hidden self-test exercises every state without taking user input.
- All 18 states were visually swept at 1280x720, 1920x1080, 960x540, and
  720x1280 after the final typography and presentation fixes.
- The optional live suite directly edits the native canvas, stages node/group
  focus links safely, and simulates view/provider state plus 36 logical/physical
  display presets, device scale, UI scale, fit, sampling, safe area, and form
  factor.
- A real Gubsy-runtime smoke validates mapped input, SDL events, typed model
  behavior, events, asset-domain lookup, display resolution, focus, and paint.

The next confidence step is a deliberately different multi-game UI suite, not
more styling of this one shell. Splonks migration remains after that review.
