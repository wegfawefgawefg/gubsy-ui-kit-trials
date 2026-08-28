# GView trial

This is the complete native Gubsy shell reference implemented with standalone
GLayout and GView over SDL3. It shares the content and behavior target used by
the Vue and RmlUi trials while exercising renderer-neutral layout, controls,
focus, scrolling, and paint commands.

```sh
./scripts/run.sh
./scripts/run.sh --screen 16
./scripts/run.sh --hidden --screen 9 --capture /tmp/gview.bmp
./scripts/run.sh --hidden --benchmark
```

Screen indices match the RmlUi trial: four Play flows, three Players tabs, four
Settings tabs, three Controls tabs, Progress, Installed Mods, and Catalog.

The window is created hidden as an SDL utility, positioned on the leftmost
display, and only then shown. Hidden capture and benchmark modes never take
pointer or keyboard focus.
