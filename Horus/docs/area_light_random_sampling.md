# Direct Lighting via Random Single-Light Sampling

This document explains the **random-light Next Event Estimation (NEE)** method discussed
for `AreaLight`: at each diffuse hit point, instead of testing every light in the scene,
you pick **one** light at random and cast a **single** shadow ray toward it. This keeps the
shadow-ray cost at `O(1)` per hit, independent of how many lights (`K`) are in the scene.

This is *additive* to the existing indirect lighting already implemented in
`Integrator::rayPath()` (`render.cpp:153-163`) — the cosine-weighted BSDF-scattered bounce
ray keeps working exactly as it does today. This document only covers the **new** direct
term you're adding alongside it.

Everything below happens once per diffuse hit point, inside `rayPath()`, using the
existing `hitPoint` and `surfaceNormal` already computed there.

---

## Step 1 — Pick a light at random

You already have `areaLights` (a `vector<AreaLight*>`) available to the `Integrator`
(via `Scene::getAreaLights()`, same pattern as the existing `lights` member).

```
K = areaLights.size()
if K == 0: skip direct lighting entirely (no lights to sample)

i = floor(unitRandom.Generate() * K)      // uniform index in [0, K)
light = areaLights[i]

p_light = 1.0 / K                          // probability this specific light was chosen
```

`unitRandom` is the same `UnitRandom` generator already used elsewhere in `render.cpp`
(`unitRandom.Generate()` returns a float in `[0,1]`).

No searching happens here — `areaLights` is a flat list you already built in
`Scene::lightCheck()`, so "picking a light" is just an array index.

---

## Step 2 — Sample a random point on that light's surface

`AreaLight` inherits `PlaneObject`, so it already has `position`, `rotation` (via the
rotation matrix `R`), `width`, and `height`. Sample a point uniformly across its rectangle
the same way `PlaneObject::rayIntersection()` already parameterizes local space
(`hrs.h:472-473`), just run in reverse — local coordinates to world, instead of world to
local:

```
r1 = unitRandom.Generate()               // in [0,1]
r2 = unitRandom.Generate()

lx = (r1 - 0.5) * light->getWidth()      // local x in [-width/2, width/2]
lz = (r2 - 0.5) * light->getHeight()     // local z in [-height/2, height/2]

lightPoint = light->position + R * Vector3D(lx, 0, lz)   // R = light's rotation matrix
```

Because you sampled **uniformly over area**, the probability density of this specific
point, measured per unit area, is constant:

```
pdf_area = 1.0 / (light->getWidth() * light->getHeight())     // = 1 / Area
```

You'll need this `pdf_area` in Step 5.

---

## Step 3 — Build the shadow ray

```
toLight   = lightPoint - hitPoint
distance  = length(toLight)
direction = normalize(toLight)

shadowRay.origin    = hitPoint + surfaceNormal * epsilon   // same epsilon-offset trick
                                                             // already used in render.cpp:160
shadowRay.direction = direction
shadowRay.tMax       = distance - epsilon                  // stop just short of the light
```

Clipping `tMax` to just short of the light point means: if `bvh.traversal()` finds nothing
before `tMax`, the light is visible. You never need to special-case "the ray hit the light
itself" vs. "the ray hit an occluder" — clipping the range makes that ambiguity disappear.

---

## Step 4 — Test occlusion

```
occluder = bvh.traversal(shadowRay, shadowRay.tMin, shadowRay.tMax)

if occluder != nullptr:
    contribution = 0          // something blocks the path to the light — skip Step 5
else:
    // proceed to Step 5, the light is visible from hitPoint
```

This is a plain BVH traversal — the same `traversal()` already used for camera/bounce rays
(`render.cpp:35`), just with a different ray and a clipped `tMax`.

---

## Step 5 — Evaluate the lighting contribution (if unoccluded)

Two cosine terms matter here, and both can make the contribution zero:

```
cos_surface = dot(surfaceNormal, direction)          // angle at the shaded point
lightNormal = light->getNormal()                     // inherited from PlaneObject
cos_light   = dot(lightNormal, -direction)            // angle at the light's surface

if cos_surface <= 0 or cos_light <= 0:
    contribution = 0    // light is behind the surface, or hitPoint is behind
                         // the light's emitting face — area lights emit from one side only
```

If both are positive, compute the light's emitted radiance using the parameters already on
`AreaLight`:

```
Le = light->getColor() * light->getIntensity() * pow(2.0, light->getExposure())
```

(`pow(2, exposure)` is the stops-based scaling covered earlier — linear `intensity`
combined with photographic `exposure`.)

Then the direct-lighting estimator for this one sample is:

```
geometric_term = (cos_surface * cos_light) / (distance * distance)

estimate = (Le * diffuseColor * diffuseGain * geometric_term) / pdf_area
```

`diffuseColor` and `diffuseGain` are the same `Surface` shader values already extracted in
`rayPath()` (`render.cpp:75-76`) — this is the Lambertian BRDF (`diffuseColor * diffuseGain
/ π`, depending on how your existing diffuse term is normalized — match whatever
convention `diffuseGain`/`diffuseColor` already use in the indirect term, so direct and
indirect stay energy-consistent).

---

## Step 6 — Rescale by the light-selection probability

Because you only picked **one** of `K` lights, you must divide by the probability of having
picked it, so that averaged over many samples the estimator is unbiased:

```
finalContribution = estimate / p_light        // p_light = 1/K, so this is estimate * K
```

This is the same idea as `pdf_area` in Step 5 — every random choice you made (which light,
which point on it) needs its probability divided out, so that on average, across many
pixel samples, you converge to the true sum over all lights and the true integral over
each light's area.

---

## Step 7 — Add to the running color

```
color += finalContribution
```

added on top of the existing indirect term already computed via the BSDF-scattered bounce
ray (`render.cpp:162`) — direct and indirect light are summed, not substituted for each
other.

---

## Full sequence, condensed

```
1. i = random index in [0, K);  p_light = 1/K
2. lightPoint = random point on areaLights[i]'s rectangle;  pdf_area = 1/Area
3. shadowRay = hitPoint -> lightPoint, tMax clipped just short of lightPoint
4. occluder = bvh.traversal(shadowRay)
5. if not occluded and cos_surface > 0 and cos_light > 0:
       estimate = Le * BRDF * cos_surface * cos_light / distance² / pdf_area
6. finalContribution = estimate / p_light
7. color += finalContribution
```

Every step involving a random choice (which light, which point on it) carries a matching
division (`/ p_light`, `/ pdf_area`) — that pairing is what keeps the estimator correct.
Losing track of either division is the most common way this kind of code silently produces
a biased (systematically too dark or too bright) image instead of just a noisy one.
