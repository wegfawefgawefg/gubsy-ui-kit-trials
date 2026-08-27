# Initial native UI trial results

The post-polish Play/Mods comparison and current recommendation are in
[`CURRENT_COMPARISON.md`](CURRENT_COMPARISON.md). The tables below preserve the
earlier baseline run.

Measured on the same machine and GPU on 2026-08-27. Each application renders
the populated Play lobby in a release build to a hidden/offscreen target for
1,000 frames. These are native-process experiments, not browser measurements.

## CPU timing

| Backend | Viewport | UI build/update | Render recording | Complete CPU frame |
|---|---:|---:|---:|---:|
| RmlUi + SDL_GPU | 1280x720 | 0.0026 ms | 0.0908 ms | 0.2066 ms |
| Dear ImGui + SDL_GPU | 1280x720 | 0.0127 ms | 0.0050 ms | 0.0956 ms |
| raygui + raylib/OpenGL | 1280x720 | combined | combined | 0.1437 ms |
| Nuklear + SDL2/OpenGL | 1280x720 | 0.0201 ms | 0.1909 ms | 0.2322 ms |
| RmlUi + SDL_GPU | 1920x1080 | 0.0027 ms | 0.0940 ms | 0.2401 ms |
| Dear ImGui + SDL_GPU | 1920x1080 | 0.0128 ms | 0.0049 ms | 0.1189 ms |
| raygui + raylib/OpenGL | 1920x1080 | combined | combined | 0.1461 ms |
| Nuklear + SDL2/OpenGL | 1920x1080 | 0.0132 ms | 0.1857 ms | 0.2185 ms |

All four are far below the 3 ms UI budget and below 1 ms in this populated
menu. The full CPU-frame figures include GPU completion waits for the hidden
benchmark where the backend exposes them. raygui's figure covers its combined
immediate draw plus frame submission. It was rerun with vsync disabled; an
earlier 6.93 ms result was only the 144 Hz present wait and was discarded.

RmlUi's Play document becomes ready in about 10.2 ms once the graphics/runtime
are initialized. Hiding and showing the resident document average 0.0048 ms and
0.0065 ms, respectively, so a resident menu can come up or down well inside one
60 Hz or 144 Hz frame.

## Memory and binary envelope

| Backend | Process RSS at 1280x720 | Release binary |
|---|---:|---:|
| RmlUi + SDL_GPU | 88,428 KiB | 8,562,032 bytes |
| Dear ImGui + SDL_GPU | 79,812 KiB | 5,041,504 bytes |
| raygui + raylib/OpenGL | 113,612 KiB | 1,505,096 bytes |
| Nuklear + SDL2/OpenGL | 116,472 KiB | 496,016 bytes |

RSS is the whole process—including SDL/raylib, Vulkan or OpenGL userspace
drivers, font atlases, swapchain/render targets, allocators, and the UI. It is
not the size of a UI object. The Vulkan and OpenGL process envelopes therefore
should not be read as direct library heap rankings. RmlUi's separately measured
resident-document delta is about 6.7 MiB for this complete shell. Equivalent
object/delta instrumentation has not yet been added to the three immediate-mode
trials.

## Functional/layout observations

- RmlUi remains the strongest authoring fit for the Vue-derived responsive
  compositions. Its custom console focus graph is the most work, but that work
  is now explicit and reusable.
- Dear ImGui is the least risky immediate-mode option. Its SDL3, SDL_GPU, and
  gamepad navigation backends work together, its widgets are complete, and the
  full shell is concise. It does not have to look like a debug inspector; the
  trial uses a borderless full-screen game shell. Responsive composition is
  still application code.
- raygui produces the smallest simple C++/raylib implementation and performs
  very well. Layout, clipping, scrolling, high-DPI scaling, and controller focus
  are largely application responsibilities. This is visible in the narrow
  captures.
- Nuklear produces the smallest binary and a complete immediate-mode control
  set, but it required the most manual row/group bookkeeping. The official
  reference backend used here is SDL2/OpenGL; SDL3/Gubsy rendering and polished
  controller focus would be additional integration work.
- 1280x720 and 1920x1080 desktop routes are usable in all implementations.
  The 915x412 and 412x915 captures are intentionally retained: they demonstrate
  that the immediate-mode trials need explicit compact list/detail and
  setup/party compositions. They are not claimed as finished phone UIs.

Raw output is in `benchmark-raw.txt`; all 72 new backend/route/viewport captures
can be regenerated with `../scripts/capture-all.sh`.
