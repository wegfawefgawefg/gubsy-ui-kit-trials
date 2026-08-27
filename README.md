# Gubsy raygui trial

An immediate-mode implementation of the shared Gubsy shell evaluation contract
using raygui 5 on raylib 5.5. It intentionally uses a plain functional theme so
the trial measures layout authoring, utility, controls, and runtime cost.

```sh
./scripts/run.sh
./scripts/run.sh --resolution 1920x1080
./scripts/run.sh --screen mods --capture /tmp/raygui-mods.png
./scripts/run.sh --benchmark 2000
```

Keyboard shortcuts: `F1`–`F6` select the six destinations. The native widgets
remain mouse/keyboard operable. Basic gamepad focus is provided by raygui/raylib;
custom console-style spatial navigation is outside this comparison pass.
