# Current native UI comparison

Measured on 2026-08-27, with GView added on 2026-08-28 after its full production
proof. Release
builds rendered 1,200 uncapped hidden frames on the same machine. `Play` is the
normal lobby workload. `Mods` is the denser workload with 20 packages, artwork,
independent scrolling regions, compatibility state, and dependency details.

## Complete CPU frame time

The figures include UI update/build, render command recording, submission, and
the backend's benchmark completion/wait path. They are useful end-to-end costs,
not a claim that all five rendering APIs expose identical timing boundaries.

| Backend | 720p Play | 720p Mods | 1080p Play | 1080p Mods |
|---|---:|---:|---:|---:|
| GView + SDL3 | 0.2562 ms | 0.2752 ms | 0.2551 ms | 0.3171 ms |
| RmlUi + SDL_GPU | 0.2312 ms | 0.2940 ms | 0.2334 ms | 0.3465 ms |
| Arbor + SDL_GPU | 0.4506 ms | 0.6708 ms | 0.5338 ms | 0.8119 ms |
| Dear ImGui + SDL_GPU | 0.0878 ms | 0.0980 ms | 0.1127 ms | 0.1215 ms |
| raygui + raylib/OpenGL | 0.1519 ms | 0.2343 ms | 0.1524 ms | 0.2334 ms |
| Nuklear + SDL2/OpenGL | 0.2659 ms | 0.1609 ms | 0.2648 ms | 0.2232 ms |

Every implementation is far below the 3 ms target and leaves ample room at
144 Hz. Nuklear's Play/Mods inversion is repeatable in this harness and appears
to come from its different mix of text/card geometry; it is not evidence that
the larger Mods screen is intrinsically cheaper.

Arbor's row is deliberately conservative: its benchmark changes one visible
binding on every sampled frame, forcing retained reconciliation, text change,
and the affected layout/render-list work. The other rows use their existing
steady populated benchmark paths. Arbor's retained UI total was 0.3242 ms for
Play and 0.5444 ms for Mods at 720p; complete-frame p95 was 0.7693 ms and
1.0007 ms respectively. Its maximum sampled complete frame remained under
2.93 ms. SDL_GPU submission timing currently reports zero in Arbor, so the
complete host render call is the useful end-to-end figure.

GView's Mods row is deliberately active rather than stable: it changes and
rebuilds the catalog's clipped paint commands every sampled frame while
scrolling. Its dense 1080p update was 0.0107 ms, renderer recording 0.1872 ms,
full-frame p99 0.3785 ms, and maximum 1.9740 ms. Stable Play built layout and
paint once across 2,000 frames. The complete scenario table is in
[`GVIEW_RESULTS.md`](GVIEW_RESULTS.md).

RmlUi separately measured retained-document update at 0.0033 ms for Play and
0.0044 ms for Mods at 720p. Initial document readiness was 13.5 ms and 34.2 ms,
respectively, but hiding/showing an already resident document took about
0.006-0.012 ms. A production game should therefore preload or retain major menu
documents rather than parse the catalog on the frame it is first requested.

## Whole-process memory and binary size

| Backend | Play RSS | Mods RSS | Release executable |
|---|---:|---:|---:|
| GView + SDL3 | 137,072 KiB | 136,308 KiB | 6,496,408 bytes |
| RmlUi + SDL_GPU | 88,160 KiB | 90,028 KiB | 8,562,032 bytes |
| Arbor + SDL_GPU | 100,608 KiB | 101,100 KiB | 3,930,080 bytes |
| Dear ImGui + SDL_GPU | 80,120 KiB | 80,016 KiB | 5,218,008 bytes |
| raygui + raylib/OpenGL | 114,232 KiB | 114,288 KiB | 1,555,952 bytes |
| Nuklear + SDL2/OpenGL | 117,296 KiB | 117,296 KiB | 662,952 bytes |

RSS is the complete process: Vulkan/OpenGL userspace drivers, SDL/raylib,
swapchain or render targets, texture/font atlases, allocators, and UI state. It
must not be interpreted as the memory occupied by one UI tree. RmlUi's measured
resident document delta was about 6.1 MiB for Play and 7.9 MiB for Mods at 720p.
GView separately estimates 139,138 bytes owned by its Play runtime and 286,466
bytes for the dense catalog at 720p. Its higher process RSS includes the SDL renderer,
driver state, textures, SDL_image, and the optional ImGui authoring suite linked
into this comparison executable; it is not a 135 MiB UI object.

## Authoring and integration assessment

### GView

Best fit for the intended long-term Gubsy ecosystem if we are willing to own a
focused game-UI layer. It reproduces the full Vue/RmlUi target without importing
a DOM/CSS runtime, while retaining S-expression and direct C++ authoring, native
custom surfaces, lightweight stable frames, semantic Gubsy input, and an editor
built over the polished native canvas. Its 36-preset simulator separates logical
viewport, physical output, device density, UI scale, safe area, and presentation
policy. Safe group-level focus authoring generates local relationships and
preserves exact re-entry without fake-data IDs. GLayout stays independently
useful for games that only need geometry.

The cost is ownership: GView has a narrower style/layout language than the web,
and future unusual interfaces must prove reusable primitives rather than grow
one-off widgets. The current mitigation is important: RmlUi/Vue remain visual
and behavioral references, the trial is comprehensive, reusable gaps are added
to GLayout/GView rather than screen code, and a diverse game-UI suite precedes
Splonks migration.

### RmlUi

Best fit for the Gubsy default menu ecosystem. RML/RCSS preserves the browser
mockup's declarative layout model, responsive compositions, asset placement,
and visual hierarchy. The result is already capable of passing as professional
game UI. Costs are a larger integration layer, explicit game/controller focus
graphs, and retained-state/event binding work. The prototype implements 17
routes plus provider states, so its code size is not directly comparable to the
six-page immediate-mode trials.

### Arbor

Arbor now implements all six destinations plus full checkpoint selection,
session-rule editing, and session-mod management with retained AXL state and
real selects, sliders, checkboxes, scrolling, text input, and tabs. Its runtime
cost is safely inside the target even under a per-frame mutation workload, and
resident host deactivate/activate is below this timer's 0.0001 ms resolution.
Initializing SDL/Vulkan and preparing all six resident documents took
170-196 ms, so production integration should construct the UI subsystem during
loading and retain its major documents.

The trial also exposed material framework gaps: no image/texture primitive,
no fallback-font stack, a much smaller-than-browser style language without
media queries or text wrapping, no authored/spatial focus graph, and fragile
composite ergonomics around slotted scoped styles and generated model-handler
names. Gamepad input works through an SDL-to-Arbor mapping, but traversal is
sequential rather than the polished local relationships in the RmlUi trial.
Arbor is viable only if we are willing to improve those internals; it is not a
drop-in replacement for the current browser-derived target yet.

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
simple menus. The parity pass required custom font-atlas handling, artwork,
custom top-layer dropdowns, full subviews, manual pointer sliders, custom
top-layer dropdowns, manual scrolling, explicit high-DPI rules, asset handling,
and eventual custom controller focus. Those are precisely the facilities the
Gubsy-wide solution needs, so choosing raygui would mean building much of a UI
framework around the widget library.

### Nuklear

Smallest executable and renderer-neutral C API, but highest authoring friction.
Basic professional presentation required manual two-line navigation, explicit
font roles, viewport-derived row/group sizing, image atlas slicing, action
wiring, local scroll decisions, and i3/X11 window handling. Controller behavior and an SDL3
Gubsy renderer remain additional work. The polished trial is now usable, but it
still looks more utilitarian and is the least attractive default ecosystem base.

## Recommendation

Proceed with GView as the leading Gubsy-owned candidate and use the new
multi-game UI suite as its next selection gate. Keep RmlUi as the strongest
ready-made retained reference and a credible fallback; it remains valuable for
checking browser-derived composition and interaction. Keep Dear ImGui as the
debug/reference implementation. Keep Arbor in the comparison as Kyle's
ergonomic/AXL experiment, but do not grow it into a second overlapping runtime;
an eventual AXL frontend can target GView if that authoring path remains useful.

Do not migrate Splonks yet. First prove GView on materially different HUD,
inventory, stylized menu, settings, and overlay compositions, adding only
general capabilities. Then integrate the reviewed result through Gubsy's typed
events, mapped input, assets, and optional authoring adapters.
Do not select raygui or Nuklear for the default ecosystem solely because their
executables are smaller; their missing framework behavior moves complexity into
Gubsy and game code.
