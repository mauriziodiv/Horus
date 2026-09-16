# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Open `Horus.sln` in Visual Studio 2022 and build with **Ctrl+Shift+B**, or from the command line:

```
msbuild Horus.sln /p:Configuration=Debug /p:Platform=x64
```

Toolset v145 (bumped from v143 in `30779a7`). Headers are under `Horus/headers/`, sources under `Horus/src/`.

**Only `Debug|x64` builds.** C++17 (`stdcpp17`) and the `IMATH_DLL;OPENEXR_DLL` defines OpenEXR needs are set on that configuration only (`Horus.vcxproj:139-142`); `Release|x64` still lacks both, so it will not compile until they are added there.

## Running

```
Horus.exe <scene_file> ppm <output.ppm> [gamma2] [window] [focal_length] [samples] [bounces]
```

- `<scene_file>` — path to a scene description file (custom text format, see below); can live anywhere on disk, passed through as-is
- `ppm` — only supported output format; lowercase, case-sensitive
- `<output.ppm>` — output file path
- `[gamma2]` — applies sqrt-based gamma correction (gamma = 2); lowercase, case-sensitive
- `[window]` — render resolution as `width,height` (e.g. `1920,1080`); overrides the camera `window` in the scene file, which is dead
- `[focal_length]` — camera focal length in mm (matches Maya's focalLength against a 24mm aperture)
- `[samples]` — samples per pixel; must be >= 1
- `[bounces]` — maximum ray depth

Arguments are strictly positional: to set a later one, every earlier one must be present. Pass an empty string `""` to skip an optional argument you don't want to set.

Every argument except the scene file accepts an optional leading `-` (e.g. `-ppm`, `-C:\path\to\output.ppm`, `-gamma2`, `-1920,1080`) which is stripped before parsing.

## Validation

No test suite. Validate changes by building, running on a scene file, and inspecting the PPM output visually.

## Scene File Format

Custom token-based text format. Objects are terminated with `;`. Parameters use `-paramName-` as delimiters and `/value/` for values. Vector values are `/x,y,z/`. Example parameters: `pos`, `rot`, `size`, `radius`, `color`, `shader`, `diffuse_gain`, `diffuse_color`, `roughness`, `refraction_gain`, `ior`, `intensity`, `lat`, `width`, `height`, `window`.

Shader types: `constant`, `depth`, `surface`.

An unknown parameter name in a `.srs` shader file **hangs the parser** rather than erroring out.

## Architecture Gotchas

- **`|` operator = cross product** — non-standard but intentional; used throughout `vec_math.h` and render code.
- **Rendering is multithreaded** — `Scene::render()` spawns `hardware_concurrency()` threads that pull scanlines off an atomic row counter (`scene.cpp:318`), each thread building its own `Integrator`. Anything reached from the ray path must be per-thread or immutable.
- **HitRecord is caller-owned** — `rayIntersection(ray, tMin, tMax, HitRecord& hit)` and `Accelerator::traversal()` fill a `HitRecord` passed in by reference (`hrs.h:152` — `front`/`back`, `b0`–`b2`, `hitPoint`, `t`). No hit state is stored on `GeometryObject`; that is what makes traversal thread-safe.
- **Normal is NOT in HitRecord** — after a hit, get it from `closestHit->computeNormal(hit)`, which *returns* the vector (`hrs.h:266`). There is no `GeometryObject::getNormal()`; the only surviving `getNormal()` is `AreaLight`'s (`hrs.h:434`).
- **Two unrelated `computeNormal`s** — `GeometryObject::computeNormal(hit)` returns the geometric normal; `Surface::computeNormal(ch, uv, normal)` (`BxDF.h:223`) applies the normal map to it. The name collision is deliberate; leave it.
- **Shader dispatch via `std::variant`** — shaders are stored as `std::variant<Shader, Constant, Depth, Surface>` and dispatched with `std::visit` in `rayPath()`. Do not add virtual methods to the shader hierarchy; extend the variant instead.
- **Scatter blend formula** (diffuse/specular path, when `refractionGain == 0`): `finalScatter = reflectedDir * (1 - roughness) + diffuseScatter * roughness`
- **Refraction uses Schlick Fresnel** — when `refractionGain > 0`, `rayPath()` stochastically chooses reflection (probability F) or refraction (probability 1-F) via the Schlick approximation; total internal reflection falls back to pure reflection.
- **`PlaneObject` bounding box** is padded with epsilon=0.001 to avoid degenerate slab test in BVH.
- **Planes are one-way** — the diffuse path never flips the shading normal toward the incoming ray, so a plane lit from behind stays black. Known and open.

## Code Style

- **Types**: `PascalCase` (e.g., `GeometryObject`, `Surface`, `BoundingBox`)
- **Methods and members**: `camelCase` (e.g., `computeNormal()`, `diffuseGain`)
- **Indentation**: tabs
- **Operator overloads**: `%` for element-wise multiply, `|` for cross product, `*` for dot product or scalar scale (context-dependent — check `vec_math.h`)
- No external formatter configured; match the style of surrounding code.
