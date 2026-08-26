# Vue parity matrix

The browser prototype is the behavioral specification. Native implementation
may differ mildly in text metrics and primitives, but not in information
architecture, state transitions, visible ownership, or responsive intent.

| Area | Required native states and behavior | Reference capture |
|---|---|---|
| Shell | Status/profile, fixed rail, bottom phone nav, prompt bar, notifications, modal layer, populated/empty/loading/error provider states | all captures |
| Play lobby | Continue/new/Arena activity payloads, checkpoint/quest, access, host method, rules, session mods, party, launch/pause | `play-lobby` |
| Quest/resume | Campaign and checkpoint selection, quest route/stages, owner/profile and payload context | `play-quest` |
| Session rules | Mode-dependent schema, typed controls, rich inspector, reset, host authority/sync, mod-sourced rows | `play-settings` |
| Session mods | Current manifest, saved-manifest differences, install/update-and-enable, dependencies, runtime policies, browse-and-add | `play-session-mods` |
| Players | Local roster, profiles/history, device ownership, multiple devices per player, ready/locked states | `players` |
| Settings | Display, audio, accessibility, gameplay; select/toggle/range/text/action widgets and detail explanation | `settings` |
| Bindings | Profiles, filter, action list/detail, multiple bindings, add/replace/remove, capture and explicit selection, conflicts | `controls-bindings` |
| Devices | Assign/unassign, many devices per player, live raw control explorer, identification | `controls-devices` |
| Input tuning | Sensitivity, deadzone, vibration, inversion, response preview | `controls-input-tuning` |
| Progress | Profile vs campaign distinction, checkpoint history, recorded exact mod set, management actions, incompatibility | `progress` |
| Installed mods | Version choice, compatibility, dependencies/dependents, guarded cascading uninstall, update | `mods-installed` |
| Mod catalog | Search, compatible-only filter, 20 realistic entries, artwork, ratings, unavailable examples, dependency install plan | `mods-catalog` |

## Viewport acceptance

| Family | CSS/native viewport | Intent |
|---|---:|---|
| Full game desktop | 1920×1080 | Spacious fixed shell with master/detail visible together |
| Compact game/tablet | 1280×720 | Dense but readable; contained lists scroll without moving the shell |
| Phone portrait | 412×915 | Larger touch text/targets, bottom nav, one-pane master/detail transitions |
| Phone landscape | 915×412 | Wide short layout designed independently; not a squashed portrait screen |

Each implemented screen must be inspected in every family. DPI scaling is a
separate input from viewport dimensions: the same logical composition must
remain readable when SDL reports a high-density drawable.
