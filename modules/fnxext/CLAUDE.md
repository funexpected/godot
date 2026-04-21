# modules/fnxext — Funexpected engine extensions

Grab-bag of Funexpected-specific additions to Godot 3.3: async primitives, ML inference, 2D rendering helpers, and an editor plugin. Not a single coherent subsystem — each class solves a discrete need from the game (private `math` repo).

## Registered classes

Registration in [register_types.cpp:23-37](register_types.cpp#L23-L37).

| Class | Extends | Purpose |
|-------|---------|---------|
| [`Future`](future.h) | Reference | Promise/future async primitive for chaining signals, timers, and threads. Methods: `then`, `wait`, `finally`, `retain`, `await`, static `start`, `yield`. See [FUTURE.md](FUTURE.md) for usage examples. |
| [`OnnxEngine`](onnx_engine.h) | Reference | ONNX model inference. Load a `.onnx` file, set input/output layers, call `run()`. Bundled runtime under [onnx_engine/](onnx_engine/). |
| [`MeshLine2D`](mesh_line_2d.h) | Node2D | 2D polyline primitive with curve support, gradient, and optional texturing. |
| [`StyleBoxBorderedTexture`](stylebox_bordered_texture.h) | StyleBoxTexture | StyleBox variant with configurable borders on top of a texture. |
| [`VisibilityController2D`](visibility_controller_2d.h) | Node2D | Viewport-based visibility culling — activates/deactivates grouped nodes when entering/leaving the screen. |
| [`ZipTool`](zip_tool.h) | Reference | Archive listing utility (read-only inspection of `.zip` contents). |

## Editor plugin

`CanvasLayersEditorPlugin` ([canvas_layers_editor_plugin.h](canvas_layers_editor_plugin.h)) is registered when the engine is built with `TOOLS_ENABLED` ([register_types.cpp:17-21](register_types.cpp#L17-L21)). Adds a UI for selecting and toggling canvas-layer visibility inside the scene editor.

## Build

Standard module — no extra SCons flags. `SCsub` compiles every `.cpp` in the module directory plus the ONNX runtime.

## Notes

- `Future` is used throughout the math game's GDScript for async flows (resource loading, animation sequencing) as an alternative to `yield` chains. Prefer reading [FUTURE.md](FUTURE.md) before touching its C++.
- `OnnxEngine` is consumed by the game for on-device inference (e.g. handwriting / character recognition).
- Nothing in this module is visible to upstream Godot — all classes are Funexpected-only.
