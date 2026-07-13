# Godot GDExtension Crash — Root Cause & Fix

Project: colony-sim-2 / world extension
Godot version: v4.5.1.stable.official
Symptom: Extension loaded, but crashed with `SIGSEGV` deep in `ClassDB::instance_binding_callbacks`
during `register_engine_classes()` — before any of the project's own code ran.

## Root Cause

godot-cpp's bindings (`gen/`, `bin/*.a`) were built against a stale or mismatched
`extension_api.json`, not the exact API of the installed engine binary. This corrupted
internal `StringName` hashing during engine class registration on extension load.

## Fix — Full Rebuild Against Correct API

```bash
cd ~/Godot/colony-sim-2/extensions/world/godot-cpp

# Dump the exact API from the actual engine binary being used
~/Godot/godot.x86_64 --headless --dump-extension-api

# Confirm extension_api.json was created
ls *.json

# Wipe old build artifacts completely
rm -rf bin gen .sconsign.dblite

# Rebuild godot-cpp bindings using the fresh dump
scons platform=linux target=template_debug custom_api_file=extension_api.json
scons platform=linux target=editor custom_api_file=extension_api.json
scons platform=linux target=template_release custom_api_file=extension_api.json
```

Then rebuild the extension itself:

```bash
cd ~/Godot/colony-sim-2/extensions/world/build
rm -rf *
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF
cmake --build .
```

Verified fix by running under gdb — full session completed (benchmark, tile placement,
serialization, clean exit), no crash.

---

## Everything Tried, In Order (Diagnostic Log)

1. **Initial build setup** — confirmed CMakeLists.txt requires godot-cpp built first:
   ```bash
   cd godot-cpp
   scons platform=linux target=template_debug   # or template_release / editor
   ```
   Then configure + build the extension:
   ```bash
   mkdir -p build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug
   cmake --build .
   ```

2. **Case-sensitivity bug** — `cmake .. -DCMAKE_BUILD_TYPE=Editor` failed because
   CMakeLists.txt checks lowercase `"editor"` in a `STREQUAL`. Fixed by using
   `-DCMAKE_BUILD_TYPE=editor`.

3. **Missing godot-cpp static lib** — build failed with
   `No rule to make target '.../libgodot-cpp.linux.template_release.x86_64.a'`
   because godot-cpp had never been compiled. Fixed by running the scons build
   (step 1) before the CMake build.

4. **Building only `world`, not tests** — `BUILD_TESTS` defaults ON and also
   builds `test_serialization`. To build only the main library:
   ```bash
   cmake --build . --target world
   ```
   Or disable tests at configure time:
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=editor -DBUILD_TESTS=OFF
   ```

5. **Godot not launching, no error shown in editor GUI** — ran engine from
   terminal to see real errors:
   ```bash
   ~/Godot/godot.x86_64 --verbose --path ~/Godot/colony-sim-2
   ```

6. **Duplicate `.gdextension` files** — found three copies
   (`world.gdextension` at project root, in `bin/`, and in `extensions/world/`)
   with inconsistent `[libraries]` keys (one was missing `linux.editor.x86_64`).
   Deleted the duplicates, kept a single canonical file at project root with all
   three keys:
   ```ini
   [configuration]
   entry_symbol = "gdextension_initialize"
   compatibility_minimum = "4.5"

   [libraries]
   linux.debug.x86_64 = "res://bin/libworld.so"
   linux.release.x86_64 = "res://bin/libworld.so"
   linux.editor.x86_64 = "res://bin/libworld.so"
   ```

7. **First real crash trace** — after fixing the `.gdextension` issue, Godot loaded
   the extension but segfaulted (signal 11). Backtrace only showed addresses, no
   symbols, because the release/no-debug lib was used.

8. **Rebuilt with debug symbols:**
   ```bash
   cd extensions/world/build
   rm -rf *
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF
   cmake --build .
   ```

9. **Ran under gdb** to get a real backtrace:
   ```bash
   gdb --args ~/Godot/godot.x86_64 --path ~/Godot/colony-sim-2
   run
   bt full
   ```
   Trace showed crash in `godot::StringName::hash()` called from
   `AHashMap<StringName, ...>::operator[]` on `ClassDB::instance_binding_callbacks`,
   during `register_engine_class()` inside `register_engine_classes()` —
   i.e. inside godot-cpp's own startup, before `World` class registration.

10. **Ruled out suspects along the way:**
    - `perlinNoise.h` had a file-scope `static const BiomeData BIOME_TABLE[...]`
      — checked, but `BiomeData` is plain floats only, not a Godot type. Not the cause.
    - `ProxyManager.h` / `hash_utils.h` — reviewed, no premature Godot-type statics.
    - `world.h` / `world.cpp` `_bind_methods()` — reviewed, bindings well-formed.
    - godot-cpp branch — confirmed on `4.5` branch, matching engine's `4.5.1`
      major/minor. Close but not verified against the *exact* patch build.
    - Duplicate `.gdextension` files — already fixed in step 6, re-confirmed only
      one file existed (`find ~/Godot/colony-sim-2 -iname "world.gdextension"`).

11. **Identified true root cause** — crash was in godot-cpp's own engine class
    registration, meaning the bindings themselves (`gen/`, `bin/*.a`) were stale
    relative to the exact running engine binary. Fixed via full API dump + full
    rebuild (see "Fix" section above).

---

## How To Update .cpp Files and Rebuild (Normal Workflow)

Once the environment is in a known-good state, day-to-day changes are simple —
no need to redo the godot-cpp API dump unless the Godot engine binary itself changes.

1. Edit your `.cpp` / `.h` files as normal in `extensions/world/`.

2. Rebuild just the `world` target:
   ```bash
   cd ~/Godot/colony-sim-2/extensions/world/build
   cmake --build . --target world
   ```
   This recompiles only changed files and relinks `libworld.so` into
   `~/Godot/colony-sim-2/bin/`.

3. If you added/removed a `.cpp` file, update the `SOURCES` list in
   `CMakeLists.txt` first, then re-run cmake configure before building:
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF
   cmake --build . --target world
   ```

4. Relaunch Godot to pick up the new `.so`:
   ```bash
   ~/Godot/godot.x86_64 --path ~/Godot/colony-sim-2
   ```
   (Godot editor also auto-reloads the extension if already open, but a fresh
   launch is safest after a crash-prone change.)

5. If something crashes again, go straight to gdb instead of guessing:
   ```bash
   gdb --args ~/Godot/godot.x86_64 --path ~/Godot/colony-sim-2
   run
   bt full
   ```

### When to redo the full godot-cpp rebuild (step 1 of the Fix section)

Only needed when:
- The Godot engine binary is updated (even a patch version bump)
- godot-cpp is updated/re-cloned to a different commit or branch
- You see the exact `ClassDB::instance_binding_callbacks` / `StringName::hash()`
  crash signature again

Otherwise, the normal incremental rebuild (steps 1–5 above) is all that's needed.