# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Open `Horus.sln` in Visual Studio 2022 and build with **Ctrl+Shift+B**, or from the command line:

```
msbuild Horus.sln /p:Configuration=Debug /p:Platform=x64
```

Configurations: `Debug|x64` (development) and `Release|x64` (performance). C++17 (`stdcpp17`), toolset v143. Headers are under `Horus/headers/`, sources under `Horus/src/`.

## Running

```
Horus.exe <scene_file> PPM <output.ppm> [GAMMA2]
```

- `<scene_file>` — path to a scene description file (custom text format, see below)
- `PPM` — only supported output format
- `[GAMMA2]` — optional; applies sqrt-based gamma correction (gamma = 2)

## Validation

No test suite. Validate changes by building, running on a scene file, and inspecting the PPM output visually.

## Scene File Format

Custom token-based text format. Objects are terminated with `;`. Parameters use `-paramName-` as delimiters and `/value/` for values. Vector values are `/x,y,z/`. Example parameters: `pos`, `rot`, `size`, `radius`, `color`, `shader`, `diffuse_gain`, `diffuse_color`, `roughness`, `refraction_gain`, `ior`, `intensity`, `lat`, `width`, `height`, `window`.

Shader types: `constant`, `depth`, `surface`.

## Architecture Gotchas

- **`|` operator = cross product** — non-standard but intentional; used throughout `vec_math.h` and render code.
- **HitRecord lives on GeometryObject** — `hitRecord` is a member of `GeometryObject`, not returned by value. This makes traversal non-thread-safe as-is.
- **Normal is NOT in HitRecord** — after a hit, call `geometry->computeNormal()` then `geometry->getNormal()` explicitly; do not assume the normal is set.
- **Shader dispatch via `std::variant`** — shaders are stored as `std::variant<Shader, Constant, Depth, Surface>` and dispatched with `std::visit` in `rayPath()`. Do not add virtual methods to the shader hierarchy; extend the variant instead.
- **Scatter blend formula** (diffuse/specular path, when `refractionGain == 0`): `finalScatter = reflectedDir * (1 - roughness) + diffuseScatter * roughness`
- **Refraction uses Schlick Fresnel** — when `refractionGain > 0`, `rayPath()` stochastically chooses reflection (probability F) or refraction (probability 1-F) via the Schlick approximation; total internal reflection falls back to pure reflection.
- **`PlaneObject` bounding box** is padded with epsilon=0.001 to avoid degenerate slab test in BVH.

## Code Style

- **Types**: `PascalCase` (e.g., `GeometryObject`, `Surface`, `BoundingBox`)
- **Methods and members**: `camelCase` (e.g., `computeNormal()`, `diffuseGain`)
- **Indentation**: tabs
- **Operator overloads**: `%` for element-wise multiply, `|` for cross product, `*` for dot product or scalar scale (context-dependent — check `vec_math.h`)
- No external formatter configured; match the style of surrounding code.
