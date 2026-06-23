# Funexpected Godot — custom engine fork

Funexpected's proprietary fork of Godot 3.3+. C++ engine used by Funexpected Math (the private `math` repo) on iOS, Android, and Web.

This repo only contains the engine. The game client, tooling, backend and scripts live in the private sister repo `math`.

## What's customized vs upstream Godot 3.3

| Area | Customization |
|------|---------------|
| `modules/flash/` | Adobe Animate playback (`.zfl` import, `FlashPlayer` Node2D). Separate public repo: [funexpected/godot-flash-module](https://github.com/funexpected/godot-flash-module). See its `CLAUDE.md`. |
| `modules/spine/` | Spine 2D skeletal animation (multi-version runtime: 3.6–4.1). Separate public repo: [funexpected/godot-spine-module](https://github.com/funexpected/godot-spine-module). See its `CLAUDE.md`. |
| `modules/fnxext/` | Funexpected extensions: `Future` (async), `OnnxEngine` (ML), `MeshLine2D`, visibility culling, canvas-layers editor plugin. In-tree — see [modules/fnxext/CLAUDE.md](modules/fnxext/CLAUDE.md). |
| `platform/iphone/` | Orientation handling fixes, tablet horizontal-start, windowed-mode fix (`[funexpected]`, `[funexpected.mobile]` commits). |
| `platform/android/` | NDK 27 / SDK 35 support, build fixes (`[funexpected.android]` commits). |
| `platform/osx/` | Build fixes for newer macOS (`[funexpected]` commits). |
| Debugger | `ScriptDebuggerStack` (`[fnx]` commits) — default debugger for IDE integration. |

## Repo layout

Standard Godot 3.3 tree: `core/ editor/ drivers/ main/ modules/ platform/ scene/ servers/ thirdparty/ doc/`. SCons build system (`SConstruct`). All Funexpected code is confined to the three custom modules plus platform-specific patches.

## Build

Standard Godot SCons. Examples:

```bash
# iOS release template
scons platform=iphone tools=no target=release

# Android debug template, with Spine 3.8 runtime
scons platform=android target=debug spine_runtime_3_8=yes

# Editor for Linux
scons platform=x11 tools=yes target=debug
```

Module-specific build flags (e.g. Spine runtime versions) are documented in each module's `CLAUDE.md`.

## Branches

- `master` — mirrors upstream Godot 3.3 branch; Funexpected fixes are cherry-picked on top.
- `react` — current working branch used by the game.
- Feature branches — short-lived; merged via PR with the commit conventions below.

## Commit conventions

Format: `[tag] lowercase description (#PR_NUMBER)`.

Tags seen in this repo:
- `[funexpected]` — generic fork maintenance (build, platform fixes).
- `[funexpected.mobile]` — iOS/Android behavior shared across mobile.
- `[funexpected.android]` — Android-specific (NDK/SDK, manifests).
- `[funexpected.flash]` — `modules/flash` changes.
- `[fnx]` — `modules/fnxext` (ONNX, Future, debugger).

Match the style used in the sister `math` repo's `CLAUDE.md`.

## Related repos

- `math` (private) — game client, backend, CI, tooling. Consumes this engine via its `engine/` submodule.
- `futuramath` (private) — related project.
- [funexpected/godot-flash-module](https://github.com/funexpected/godot-flash-module) — the `modules/flash` submodule.
- [funexpected/godot-spine-module](https://github.com/funexpected/godot-spine-module) — the `modules/spine` submodule.

## Upstream

Based on [Godot 3.3.x](https://github.com/godotengine/godot/tree/3.3). Refer to upstream docs for non-custom subsystems (servers, scene tree, GDScript 1.x).
