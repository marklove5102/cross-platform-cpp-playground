# Hello World — C++23 Modules with Clang on Windows

A minimal C++23 project that demonstrates **C++ modules** (`import std;` and a custom module) compiled with Clang (via MSYS2) on Windows.

## Project Structure

```
with-clang/
├── .vscode/
│   ├── launch.json        # VS Code debug configuration
│   └── tasks.json         # VS Code build tasks (5-step module build)
├── .gitignore             # Ignores build artifacts (*.o, *.exe, *.pcm, etc.)
├── main.cpp               # Entry point — imports std and mod modules
├── mod.cppm               # Custom module interface unit
└── README.md
```

## Prerequisites

### 1. Install MSYS2

Download and install MSYS2 from [https://www.msys2.org/](https://www.msys2.org/). The default installation path is `C:\msys64`.

### 2. Install the Clang Toolchain (UCRT64)

Open the **MSYS2 UCRT64** terminal (not the default MSYS2 terminal) and run:

```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb
```

This installs `clang++.exe` under `C:\msys64\ucrt64\bin\`. GDB is also installed for debugging (LLDB is not packaged for UCRT64 in MSYS2, but GDB works fine with Clang-compiled binaries).

> **Note:** The Clang package in MSYS2 UCRT64 uses **libstdc++** (GCC's standard library) as its default C++ standard library, not libc++. This is important for `import std;` — we compile GCC's `bits/std.cc` module source with Clang.

### 3. Add UCRT64 to your PATH

Add `C:\msys64\ucrt64\bin` to your Windows system PATH so that `clang++` and `gdb` are available from any terminal:

1. Open **Settings → System → About → Advanced system settings → Environment Variables**.
2. Under **System variables**, select `Path` and click **Edit**.
3. Click **New** and add `C:\msys64\ucrt64\bin`.
4. Click **OK** to save.

Verify it works by opening a new terminal and running:

```powershell
clang++ --version
```

You should see output like `clang version 21.1.7` (or a similar recent version).

### 4. Install VS Code Extensions (optional, for IDE support)

Install the [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) by Microsoft for IntelliSense, debugging, and build task integration.

## How C++ Modules Work with Clang

Clang's module compilation model is a **two-phase process** for each module:

1. **Precompile** the module interface unit (`.cppm`) into a **precompiled module file** (`.pcm`) using the `--precompile` flag.
2. **Compile** the `.pcm` into a regular **object file** (`.o`) using `-c`.

The `.pcm` file is what allows other translation units to `import` the module. The `.o` file contains the compiled code needed at link time. Both are required.

When a source file imports a module, you must tell Clang where to find the `.pcm` using the `-fmodule-file=<name>=<path>` flag. For example, `-fmodule-file=std=std.pcm` tells Clang that `import std;` should resolve to `std.pcm`.

### How `import std;` Works

Like GCC, **Clang does not ship pre-compiled standard library modules**. Since the MSYS2 UCRT64 Clang toolchain uses libstdc++, we compile GCC's module source file directly with Clang:

```
C:\msys64\ucrt64\include\c++\<version>\bits\std.cc
```

> **Note:** Replace `<version>` with your installed GCC version (e.g., `15.2.0`). You can find it by checking which version directories exist under `C:\msys64\ucrt64\include\c++\`.

Because `std.cc` has a `.cc` extension (not `.cppm`), Clang doesn't automatically recognize it as a module interface unit. We must explicitly tell Clang to treat it as one using `-x c++-module`.

## Building from the Command Line

All commands below should be run from the `with-clang/` directory.

### Step 1: Precompile the `std` Module Interface

```powershell
clang++ -std=c++23 -x c++-module --precompile -g -Wno-reserved-module-identifier "C:\msys64\ucrt64\include\c++\15.2.0\bits\std.cc" -o std.pcm
```

This produces `std.pcm` — the precompiled module interface for the entire C++ standard library.

**Flags explained:**
- `-x c++-module` — tells Clang to treat the input as a module interface unit (needed because the file extension is `.cc`, not `.cppm`).
- `--precompile` — produce a `.pcm` file instead of compiling to an object file.
- `-Wno-reserved-module-identifier` — suppresses the warning that `std` is a reserved module name (it is reserved, and we're intentionally using it).

> **Important:** This step can take several seconds as it precompiles the entire C++ standard library. You only need to redo this if you change your Clang or GCC version, or delete the `.pcm` file.

### Step 2: Compile the `std` Module to an Object File

```powershell
clang++ -c -g std.pcm -o std.o
```

This compiles the precompiled module into a regular object file (`std.o`) that the linker needs.

### Step 3: Precompile Your Custom Module(s)

```powershell
clang++ -std=c++23 --precompile -g "-fmodule-file=std=std.pcm" mod.cppm -o mod.pcm
```

This precompiles `mod.cppm` into `mod.pcm`. The `-fmodule-file=std=std.pcm` flag is included so that if `mod.cppm` uses `import std;` internally, Clang knows where to find it.

> **Note:** In PowerShell, the `-fmodule-file=...` argument must be quoted (with double quotes) because PowerShell interprets the `=` sign specially.

### Step 4: Compile Your Custom Module to an Object File

```powershell
clang++ -c -g mod.pcm -o mod.o
```

### Step 5: Compile and Link the Main Program

```powershell
clang++ -std=c++23 -g "-fmodule-file=std=std.pcm" "-fmodule-file=mod=mod.pcm" main.cpp std.o mod.o -o main.exe
```

This compiles `main.cpp` (resolving `import std;` and `import mod;` via the `.pcm` files), links it against both module object files, and produces the final `main.exe`.

### Step 6: Run It

```powershell
.\main.exe
```

Expected output:

```
Hello C++ World from VS Code and the C++ extension!
```

### All Steps Combined

For convenience, you can run the entire build as a single chained command:

```powershell
clang++ -std=c++23 -x c++-module --precompile -g -Wno-reserved-module-identifier "C:\msys64\ucrt64\include\c++\15.2.0\bits\std.cc" -o std.pcm && clang++ -c -g std.pcm -o std.o && clang++ -std=c++23 --precompile -g "-fmodule-file=std=std.pcm" mod.cppm -o mod.pcm && clang++ -c -g mod.pcm -o mod.o && clang++ -std=c++23 -g "-fmodule-file=std=std.pcm" "-fmodule-file=mod=mod.pcm" main.cpp std.o mod.o -o main.exe
```

## Building from VS Code

The included `.vscode/tasks.json` defines five build tasks chained together with `dependsOn`, so you only need to trigger one:

1. Press **Ctrl+Shift+B** to run the default build task.
2. VS Code will automatically execute the tasks in order:
   - **Precompile std module** → precompiles `std.cc` into `std.pcm`
   - **Compile std module object** → compiles `std.pcm` into `std.o`
   - **Precompile project modules** → precompiles `mod.cppm` into `mod.pcm`
   - **Compile project module objects** → compiles `mod.pcm` into `mod.o`
   - **Build and link** → compiles `main.cpp` and links everything into `main.exe`

To debug, press **F5**. The `launch.json` is configured to build first (via the pre-launch task) and then launch `main.exe` under GDB.

## Clean Build

To do a clean rebuild, delete the generated artifacts and rebuild:

```powershell
Remove-Item -Force *.pcm, *.o, main.exe
```

Then follow the build steps above again.

## Key Compiler Flags

| Flag | Purpose |
|------|---------|
| `-std=c++23` | Enables the C++23 standard, which includes module support |
| `--precompile` | Precompile a module interface unit into a `.pcm` file (Clang-specific) |
| `-x c++-module` | Force Clang to treat the input file as a module interface unit, regardless of file extension |
| `-fmodule-file=<name>=<path>` | Tell Clang where to find a precompiled module — e.g., `-fmodule-file=std=std.pcm` maps `import std;` to `std.pcm` |
| `-Wno-reserved-module-identifier` | Suppress the warning about using reserved module names like `std` |
| `-c` | Compile only (produce `.o` object file), do not link |
| `-g` | Include debug symbols for use with GDB |

## Differences from the GCC Sample

| Aspect | GCC | Clang |
|--------|-----|-------|
| Module flag | `-fmodules-ts` | Not needed (modules enabled by default with `-std=c++23`) |
| Precompiled module format | `.gcm` files in `gcm.cache/` directory | `.pcm` files in the build directory |
| Compilation steps per module | 1 step (`-c` produces both `.o` and `.gcm`) | 2 steps (`--precompile` → `.pcm`, then `-c` → `.o`) |
| Module resolution | Automatic via `gcm.cache/` directory lookup | Explicit via `-fmodule-file=<name>=<path>` flags |
| `std.cc` handling | Recognized automatically with `-fmodules-ts` | Requires `-x c++-module` to treat `.cc` as a module interface |

## Troubleshooting

### `fatal error: module 'std' not found`

Make sure you have precompiled `std.pcm` (Step 1) and are passing `-fmodule-file=std=std.pcm` to all subsequent compilation commands that encounter `import std;`.

### `error: file 'X.pcm' is not a valid precompiled file`

This usually means the `.pcm` was compiled with a different Clang version or was corrupted. Delete all `.pcm` files and rebuild from Step 1.

### `warning: 'std' is a reserved name for a module`

This is harmless — it's Clang warning that `std` is a reserved module identifier, which is expected since we're compiling the actual standard library module. The `-Wno-reserved-module-identifier` flag suppresses it.

### PowerShell argument parsing issues

PowerShell can mangle arguments containing `=` signs when running commands directly. Always wrap `-fmodule-file=...` arguments in double quotes when running from the command line:

```powershell
# Wrong — PowerShell may split this incorrectly
clang++ -fmodule-file=std=std.pcm main.cpp

# Correct — quoted to preserve the argument
clang++ "-fmodule-file=std=std.pcm" main.cpp
```

> **Note:** The VS Code build tasks avoid this issue by using `"type": "process"` instead of `"type": "shell"`, which passes arguments directly to the compiler without going through PowerShell.

### IntelliSense errors in VS Code

The C/C++ extension's IntelliSense engine may not fully support C++ modules yet. If you see red squiggles on `import` statements but the code compiles fine, you can add this to your `.vscode/settings.json` to suppress them:

```json
{
    "C_Cpp.errorSquiggles": "disabled"
}
```
