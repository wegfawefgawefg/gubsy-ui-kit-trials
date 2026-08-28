# GView trial

This is the complete native Gubsy shell reference implemented with standalone
GLayout and GView over SDL3. It shares the content and behavior target used by
the Vue and RmlUi trials while exercising renderer-neutral layout, controls,
focus, scrolling, and paint commands.

```sh
./scripts/run.sh
./scripts/run.sh --screen 16
./scripts/run.sh --screen 17
./scripts/run.sh --editor
./scripts/run.sh --hidden --screen 9 --capture /tmp/gview.bmp
./scripts/run.sh --hidden --benchmark --scenario scroll --screen 16
```

Screen indices are:

| Index | State |
|---:|---|
| 0-3 | Play lobby, quest picker, expedition rules, session mods |
| 4-6 | Local players, profiles, devices |
| 7-10 | Display, audio, accessibility, gameplay settings |
| 11-13 | Bindings, control devices, input tuning |
| 14 | Progress, campaigns, checkpoints, recorded package set |
| 15-16 | Installed mods and catalog |
| 17 | Non-menu inventory over a native custom world surface |

Every overflow region is an internal GView scroll area. The shell has no outer
document scroll. Widths below 1000 or heights below 600 select the compact
composition used for tablet, landscape phone, and portrait evidence.

Mouse controls use normal hit testing. Keyboard and gamepads produce semantic
navigation actions; controllers are opened on startup and hot-plugged. Focus
groups provide local geometric movement, remembered re-entry, explicit edge
overrides, dropdown capture/cancel, and Back-to-owner behavior instead of
moving a synthetic pointer.

F3 toggles the live authoring suite. It can switch every state and provider
scenario, simulate internal resolution/DPI/form factor/safe area, edit the
hierarchy and presentation, drag the overlay, undo/redo/save/reload, inspect
timings, and edit/validate the directed focus graph. Authoring uses the same
View and S-expression representation as runtime.

Benchmark scenarios are `stable`, `value`, `layout`, and `scroll`. The first
120 frames are discarded. Output separates runtime update, SDL recording,
complete CPU frame, compile/activation, cache builds, and GView-owned memory.

The window is created hidden as an SDL utility, positioned on the leftmost
display, and only then shown. Hidden capture and benchmark modes never take
pointer or keyboard focus.
