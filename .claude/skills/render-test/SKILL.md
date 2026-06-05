---
name: render-test
description: Build Horus via MSBuild and run it on a scene file to validate changes. Reports compile errors or describes what the output image looks like. Use after making code changes to check that things still work.
disable-model-invocation: false
---

When invoked as `/render-test [scene_file] [output_name]`:

1. **Build** — run MSBuild on `Horus.sln` targeting `Debug|x64`:
   ```
   msbuild Horus.sln /p:Configuration=Debug /p:Platform=x64 /m
   ```
   If the build fails, report the compile errors and stop. Do not run the executable.

2. **Run** — if the build succeeds, run the executable. Use the scene file from `$ARGUMENTS` if provided, otherwise ask the user which scene file to use. Default output path is `test_output.ppm`. If `GAMMA2` should be applied, append it as the fourth argument.
   ```
   x64\Debug\Horus.exe <scene_file> PPM <output.ppm> [GAMMA2]
   ```

3. **Report** — describe what happened:
   - If a compile error occurred: paste the relevant error lines and suggest the likely fix.
   - If the run succeeded: note the output file path and describe any observable changes relative to what the user was working on (e.g., "refraction now bends rays through the sphere", "background color changed").
   - If the run crashed or produced an empty/black image: report the issue and diagnose.

Keep the report concise — one paragraph unless there are errors to show.
