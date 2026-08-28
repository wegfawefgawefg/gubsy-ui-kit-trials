# GView production-proof results

Measured on 2026-08-28 from the release GView trial. Each run rendered 2,000
hidden uncapped frames after discarding 120 warmup frames. `stable`, `value`,
and `layout` use the Play lobby. `scroll` uses the 20-package catalog and changes
its internal scroll offset every frame.

## CPU frame cost

| Viewport | Scenario | Update | Render recording | Full CPU frame | p99 | Max |
|---|---|---:|---:|---:|---:|---:|
| 1280x720 | Stable | 0.0001 ms | 0.1302 ms | 0.2454 ms | 0.3435 ms | 0.5300 ms |
| 1280x720 | Value dirty | 0.0059 ms | 0.1271 ms | 0.2457 ms | 0.3177 ms | 0.5558 ms |
| 1280x720 | Layout dirty | 0.0070 ms | 0.1276 ms | 0.2481 ms | 0.3174 ms | 0.5187 ms |
| 1280x720 | Dense scroll | 0.0075 ms | 0.1443 ms | 0.2692 ms | 0.2956 ms | 0.6798 ms |
| 1920x1080 | Stable | <0.0001 ms | 0.1241 ms | 0.2321 ms | 0.2633 ms | 0.4642 ms |
| 1920x1080 | Value dirty | 0.0056 ms | 0.1238 ms | 0.2393 ms | 0.2791 ms | 0.6810 ms |
| 1920x1080 | Layout dirty | 0.0069 ms | 0.1262 ms | 0.2468 ms | 0.2871 ms | 0.9560 ms |
| 1920x1080 | Dense scroll | 0.0083 ms | 0.1667 ms | 0.2932 ms | 0.3429 ms | 1.1148 ms |

Compile took 0.015-0.029 ms and runtime activation took 0.0055-0.0083 ms.
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
| 720p Play | 96,016 bytes | 135,752 KiB |
| 720p dense catalog | 197,472 bytes | 137,456 KiB |
| 1080p Play | 96,018 bytes | 135,972 KiB |
| 1080p dense catalog | 198,111 bytes | 136,588 KiB |

The release trial executable is 6,248,200 bytes. It statically includes the
optional Dear ImGui authoring UI and SDL_image in addition to GView, GLayout,
GSEXP, and the full 18-state trial. It is not the size of `gview::core`.
For scale, a separate optimized core-only build produced a 427,074-byte static
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
- 720p, 1080p, compact landscape, and portrait compositions were visually swept.
- The optional live suite edits hierarchy/layout/presentation and the directed
  focus graph while simulating view, provider state, resolution, DPI, form
  factor, and safe area.

The next confidence step is a deliberately different multi-game UI suite, not
more styling of this one shell. Splonks migration remains after that review.
