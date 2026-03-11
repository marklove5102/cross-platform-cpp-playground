# Hello World — C++23 Modules with Clang and CMake on Windows

A minimal C++23 project that demonstrates **C++ modules** (`import std;` and a custom module) compiled with Clang (via MSYS2), built using **CMake** with the **Ninja** generator on Windows.

The key advantage of CMake over manual compilation is that CMake **automatically handles module dependency scanning and compilation ordering** — you don't need to manually precompile each module in the right order. You simply list your module files in `CMakeLists.txt`, and CMake figures out the rest.

## Project Structure

```
with-clang-cmake/
├── .vscode/
│   ├── launch.json         # VS Code debug configuration
│   └── tasks.json          # VS Code build tasks (configure + build)
├── .gitignore              # Ignores build/ directory and build artifacts
├── CMakeLists.txt          # CMake build configuration
├── main.cpp                # Entry point — imports std and mod modules
├── mod.cppm                # Custom module interface unit
└── README.md
```

## Prerequisites

### 1. Install MSYS2

Download and install MSYS2 from [https://www.msys2.org/](https://www.msys2.org/). The default installation path is `C:\msys64`.

### 2. Install the Clang Toolchain and Ninja (UCRT64)

Open the **MSYS2 UCRT64** terminal (not the default MSYS2 terminal) and run:

```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-ninja
```

This installs:
- `clang++.exe` — the Clang C++ compiler
- `gdb.exe` — the GNU debugger (LLDB is not packaged for UCRT64 in MSYS2, but GDB works fine with Clang-compiled binaries)
- `ninja.exe` — the Ninja build system, required by CMake for C++ module dependency scanning

> **Note:** The Clang package in MSYS2 UCRT64 uses **libstdc++** (GCC's standard library) as its default C++ standard library, not libc++. This is important for `import std;` — CMake compiles GCC's `bits/std.cc` module source with Clang automatically.

### 3. Install CMake

Download and install CMake ≥ 3.30 from [https://cmake.org/download/](https://cmake.org/download/). Version 3.30 is the minimum required for `import std;` support. This sample was tested with CMake 4.2.

After installation, verify it is available:

```powershell
cmake --version
```

### 4. Add UCRT64 to your PATH

Add `C:\msys64\ucrt64\bin` to your Windows system PATH so that `clang++`, `gdb`, and `ninja` are available from any terminal:

1. Open **Settings → System → About → Advanced system settings → Environment Variables**.
2. Under **System variables**, select `Path` and click **Edit**.
3. Click **New** and add `C:\msys64\ucrt64\bin`.
4. Click **OK** to save.

Verify it works by opening a new terminal and running:

```powershell
clang++ --version
ninja --version
```

### 5. Install VS Code Extensions (optional, for IDE support)

Install the [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) by Microsoft for IntelliSense, debugging, and build task integration.

## How CMake Handles C++ Modules

Unlike the manual Clang build (see the `with-clang` sample), CMake automates the entire module compilation process:

1. **Dependency scanning** — CMake (via Ninja) scans all source files for `import` and `export module` statements to determine the module dependency graph.
2. **Automatic ordering** — modules are compiled in the correct dependency order without you having to specify it.
3. **`import std;` support** — when `CMAKE_CXX_MODULE_STD` is enabled, CMake automatically locates and compiles the standard library module. It finds the module source files via a `libstdc++.modules.json` (or `libc++.modules.json`) manifest that ships with the compiler toolchain.

This means your `CMakeLists.txt` only needs to declare **which files are module interface units** — CMake handles everything else.

### Why Ninja is Required

CMake's C++ module support relies on a two-phase build process: first scanning source files for module dependencies, then building in the determined order. Currently, only the **Ninja** generator (≥ 1.11) supports this workflow. The "MinGW Makefiles" and "Unix Makefiles" generators do **not** support C++ module dependency scanning.

### The `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` Gate

As of CMake 4.2, `import std;` support is still considered **experimental**. To enable it, you must set a version-specific UUID before the `project()` call:

```cmake
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
```

This UUID is tied to the specific CMake version. If you upgrade CMake and the build fails with a message about an incorrect UUID, you will need to find the updated UUID for your CMake version. The UUID can be found in CMake's source tree under `Help/dev/experimental.rst`, or by searching the CMake binary:

```powershell
python -c "import re; data=open(r'C:\Program Files\CMake\bin\cmake.exe','rb').read(); idx=data.find(b'CxxImportStd'); [print(u.decode()) for u in re.findall(rb'[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}', data[max(0,idx-200):idx+200])]"
```

Once `import std;` graduates from experimental status in a future CMake release, this line can be removed.

## Understanding the CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(helloworld LANGUAGES CXX)

add_executable(main main.cpp)
target_sources(main PUBLIC FILE_SET CXX_MODULES FILES mod.cppm)
```

| Line | Purpose |
|------|---------|
| `cmake_minimum_required(VERSION 3.30)` | CMake 3.30 is the minimum version that supports `CMAKE_CXX_MODULE_STD` |
| `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` | Enables the experimental `import std;` feature gate (UUID is version-specific) |
| `CMAKE_CXX_MODULE_STD ON` | Tells CMake to provide the `std` module for targets that use C++23 or later |
| `CMAKE_CXX_STANDARD 23` | Sets the C++ standard to C++23 |
| `CMAKE_CXX_STANDARD_REQUIRED ON` | Fails the build if the compiler doesn't support C++23 |
| `project(helloworld LANGUAGES CXX)` | Declares the project — **must come after** the standard/experimental settings so CMake can probe for module support during toolchain detection |
| `add_executable(main main.cpp)` | Declares the executable target |
| `target_sources(... FILE_SET CXX_MODULES FILES mod.cppm)` | Declares `mod.cppm` as a C++ module interface unit — CMake will automatically scan it for `export module` declarations and compile it before any file that imports it |

> **Important:** The `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD`, `CMAKE_CXX_MODULE_STD`, `CMAKE_CXX_STANDARD`, and `CMAKE_CXX_STANDARD_REQUIRED` variables must all be set **before** the `project()` call. CMake probes for module support during `project()`, and it needs these variables to be set at that point.

## Building from the Command Line

All commands below should be run from the `with-clang-cmake/` directory.

### Step 1: Configure the Build

```powershell
cmake -B build -G Ninja -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/clang++.exe -DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/ninja.exe -Wno-dev
```

**Flags explained:**
- `-B build` — place all build files in a `build/` subdirectory (out-of-source build).
- `-G Ninja` — use the Ninja generator (required for C++ module support).
- `-DCMAKE_CXX_COMPILER=...` — explicitly specify the Clang compiler to use.
- `-DCMAKE_MAKE_PROGRAM=...` — explicitly specify the Ninja executable path.
- `-Wno-dev` — suppresses the CMake developer warning about experimental `import std;` support.

> **Note:** You only need to run the configure step once, or again after changing `CMakeLists.txt`. Subsequent builds will automatically re-configure if needed.

### Step 2: Build the Project

```powershell
cmake --build build
```

CMake (via Ninja) will automatically:
1. Scan all source files for module dependencies.
2. Compile the `std` module from libstdc++'s `bits/std.cc` (and `std.compat.cc`).
3. Compile your custom `mod.cppm` module.
4. Compile `main.cpp` and link everything into `build/main.exe`.

### Step 3: Run It

```powershell
.\build\main.exe
```

Expected output:

```
Hello C++ World from VS Code and the C++ extension!
```

### Both Steps Combined

```powershell
cmake -B build -G Ninja -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/clang++.exe -DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/ninja.exe -Wno-dev && cmake --build build
```

## Building from VS Code

The included `.vscode/tasks.json` defines two build tasks chained together with `dependsOn`:

1. Press **Ctrl+Shift+B** to run the default build task.
2. VS Code will automatically execute:
   - **CMake: Configure** → runs `cmake -B build -G Ninja ...`
   - **CMake: Build** → runs `cmake --build build`

To debug, press **F5**. The `launch.json` is configured to build first (via the pre-launch task) and then launch `build/main.exe` under GDB.

## Clean Build

To do a clean rebuild, delete the `build/` directory and rebuild:

```powershell
Remove-Item -Recurse -Force build
cmake -B build -G Ninja -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/clang++.exe -DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/ninja.exe -Wno-dev
cmake --build build
```

Or use CMake's built-in clean target:

```powershell
cmake --build build --target clean
cmake --build build
```

Note that `--target clean` removes compiled artifacts but keeps the configuration, so you don't need to re-run the configure step.

## Differences from the Manual Clang Sample

| Aspect | Manual (`with-clang`) | CMake (`with-clang-cmake`) |
|--------|----------------------|---------------------------|
| Build system | Direct `clang++` invocations | CMake + Ninja |
| Module dependency handling | Manual — you must compile each module in the correct order | Automatic — CMake scans and orders everything |
| `import std;` setup | Manual — precompile `bits/std.cc` yourself with `-x c++-module --precompile` | Automatic — CMake finds and compiles it via `libstdc++.modules.json` |
| Adding a new module | Add new precompile + compile steps, update `-fmodule-file=` flags everywhere | Add one line: `mod2.cppm` to the `FILE_SET CXX_MODULES` list |
| Build artifacts location | In the source directory | In a separate `build/` directory (out-of-source) |
| VS Code tasks | 5 chained tasks | 2 tasks (configure + build) |

## Troubleshooting

### `CMake Error: CMAKE_EXPERIMENTAL_CXX_IMPORT_STD is set to incorrect value`

The UUID in `CMakeLists.txt` doesn't match your CMake version. See the [UUID gate section](#the-cmake_experimental_cxx_import_std-gate) above for how to find the correct UUID for your version.

### `The "CXX_MODULE_STD" property on the target requires that the "__CMAKE::CXX23" target exist`

This means CMake couldn't set up the `import std;` infrastructure. Common causes:
- The `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` UUID is missing or incorrect.
- The `CMAKE_CXX_MODULE_STD`, `CMAKE_CXX_STANDARD`, or `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` variables are set **after** the `project()` call instead of before it.
- The compiler toolchain doesn't have a `libstdc++.modules.json` file. Verify it exists by running:
  ```powershell
  clang++ "-print-file-name=libstdc++.modules.json"
  ```

### `Could not find libstdc++.modules.json resource`

Your Clang/libstdc++ installation doesn't include the modules manifest file. This file is shipped with recent versions of GCC's libstdc++ (15+). Update your MSYS2 packages:

```bash
pacman -Syu
```

### `ninja: error: ... multiple rules generate the same target`

Delete the `build/` directory and reconfigure from scratch. This can happen when CMake's configuration gets out of sync after changing module files.

### IntelliSense errors in VS Code

The C/C++ extension's IntelliSense engine may not fully support C++ modules yet. If you see red squiggles on `import` statements but the code compiles fine, you can add this to your `.vscode/settings.json` to suppress them:

```json
{
    "C_Cpp.errorSquiggles": "disabled"
}
```
