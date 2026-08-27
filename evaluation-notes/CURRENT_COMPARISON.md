# Current native UI comparison

Measured on 2026-08-27 after the visual and interaction parity pass. Release
builds rendered 1,000 uncapped hidden frames on the same machine. `Play` is the
normal lobby workload. `Mods` is the denser workload with 20 packages, artwork,
independent scrolling regions, compatibility state, and dependency details.

## Complete CPU frame time

The figures include UI update/build, render command recording, submission, and
the backend's benchmark completion/wait path. They are useful end-to-end costs,
not a claim that all four rendering APIs expose identical timing boundaries.

| Backend | 720p Play | 720p Mods | 1080p Play | 1080p Mods |
|---|---:|---:|---:|---:|
| RmlUi + SDL_GPU | 0.2312 ms | 0.2940 ms | 0.2334 ms | 0.3465 ms |
| Dear ImGui + SDL_GPU | 0.0880 ms | 0.0933 ms | 0.1136 ms | 0.1283 ms |
| raygui + raylib/OpenGL | 0.1473 ms | 0.2286 ms | 0.1453 ms | 0.2275 ms |
| Nuklear + SDL2/OpenGL | 0.2715 ms | 0.1597 ms | 0.2872 ms | 0.1627 ms |

Every implementation is far below the 3 ms target and leaves ample room at
144 Hz. Nuklear's Play/Mods inversion is repeatable in this harness and appears
to come from its different mix of text/card geometry; it is not evidence that
the larger Mods screen is intrinsically cheaper.

RmlUi separately measured retained-document update at 0.0033 ms for Play and
0.0044 ms for Mods at 720p. Initial document readiness was 13.5 ms and 34.2 ms,
respectively, but hiding/showing an already resident document took about
0.006-0.012 ms. A production game should therefore preload or retain major menu
documents rather than parse the catalog on the frame it is first requested.

## Whole-process memory and binary size

| Backend | Play RSS | Mods RSS | Release executable |
|---|---:|---:|---:|
| RmlUi + SDL_GPU | 88,160 KiB | 90,028 KiB | 8,562,032 bytes |
| Dear ImGui + SDL_GPU | 80,096 KiB | 79,948 KiB | 5,205,664 bytes |
| raygui + raylib/OpenGL | 113,532 KiB | 113,688 KiB | 1,530,872 bytes |
| Nuklear + SDL2/OpenGL | 117,288 KiB | 117,172 KiB | 650,480 bytes |

RSS is the complete process: Vulkan/OpenGL userspace drivers, SDL/raylib,
swapchain or render targets, texture/font atlases, allocators, and UI state. It
must not be interpreted as the memory occupied by one UI tree. RmlUi's measured
resident document delta was about 6.1 MiB for Play and 7.9 MiB for Mods at 720p.

## Authoring and integration assessment

### RmlUi

Best fit for the Gubsy default menu ecosystem. RML/RCSS preserves the browser
mockup's declarative layout model, responsive compositions, asset placement,
and visual hierarchy. The result is already capable of passing as professional
game UI. Costs are a larger integration layer, explicit game/controller focus
graphs, and retained-state/event binding work. The prototype implements 17
routes plus provider states, so its code size is not directly comparable to the
six-page immediate-mode trials.

### Dear ImGui

Strongest fallback and best implementation/reference harness. Widgets,
dropdowns, sliders, clipping, scrolling, SDL3/SDL_GPU, and controller navigation
are mature. It was quick to make every page functional. Responsive composition
and visual structure are application code, however; accidental nested scrolling,
duplicate IDs, and clipped two-line rows all required deliberate correction.
It can look like a real game rather than a debug tool, but maintaining that look
requires a small Gubsy layout/component layer.

### raygui

Small, direct, and fast, with the smallest C++ executable. It is serviceable for
simple menus. The parity pass required custom font-atlas handling, custom
top-layer dropdowns, manual scrolling, explicit high-DPI rules, asset handling,
and eventual custom controller focus. Those are precisely the facilities the
Gubsy-wide solution needs, so choosing raygui would mean building much of a UI
framework around the widget library.

### Nuklear

Smallest executable and renderer-neutral C API, but highest authoring friction.
Basic professional presentation required manual two-line navigation, explicit
font roles, nested row/group sizing, image atlas slicing, action wiring, local
scroll decisions, and i3/X11 window handling. Controller behavior and an SDL3
Gubsy renderer remain additional work. The polished trial is now usable, but it
still looks more utilitarian and is the least attractive default ecosystem base.

## Recommendation

Proceed with RmlUi as the leading Gubsy menu backend and keep Dear ImGui as the
fallback/reference implementation. Performance is comfortably sufficient in
both. Use resident/preloaded RmlUi documents for complex screens and build the
Gubsy event, asset, focus, and layout-provider adapters around that boundary.
Do not select raygui or Nuklear for the default ecosystem solely because their
executables are smaller; their missing framework behavior moves complexity into
Gubsy and game code.
