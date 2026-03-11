# Hello World — C++23 Modules with GCC on Windows

A minimal C++23 project that demonstrates **C++ modules** (`import std;` and a custom module) compiled with GCC (MinGW via MSYS2) on Windows.

## Project Structure

```
with-gcc/
├── .vscode/
│   ├── launch.json        # VS Code debug configuration
│   └── tasks.json         # VS Code build tasks (3-step module build)
├── .gitignore             # Ignores build artifacts (*.o, *.exe, gcm.cache/, etc.)
├── main.cpp               # Entry point — imports std and mod modules
├── mod.cppm               # Custom module interface unit
└── README.md
```

## Prerequisites

### 1. Install MSYS2

Download and install MSYS2 from [https://www.msys2.org/](https://www.msys2.org/). The default installation path is `C:\msys64`.

### 2. Install the GCC Toolchain (UCRT64)

Open the **MSYS2 UCRT64** terminal (not the default MSYS2 terminal) and run:

```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb
```

This installs `g++.exe` and `gdb.exe` under `C:\msys64\ucrt64\bin\`.

### 3. Add UCRT64 to your PATH

Add `C:\msys64\ucrt64\bin` to your Windows system PATH so that `g++` and `gdb` are available from any terminal:

1. Open **Settings → System → About → Advanced system settings → Environment Variables**.
2. Under **System variables**, select `Path` and click **Edit**.
3. Click **New** and add `C:\msys64\ucrt64\bin`.
4. Click **OK** to save.

Verify it works by opening a new terminal and running:

```powershell
g++ --version
```

You should see output like `g++.exe (Rev8, Built by MSYS2 project) 15.2.0` (or a similar version ≥ 15).

### 4. Install VS Code Extensions (optional, for IDE support)

Install the [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) by Microsoft for IntelliSense, debugging, and build task integration.

## How C++ Modules Work with GCC

Unlike MSVC, **GCC does not ship pre-compiled standard library modules**. You must compile the `std` module yourself before any source file can `import std;`.

GCC ships the module source file at:

```
C:\msys64\ucrt64\include\c++\<version>\bits\std.cc
```

> **Note:** Replace `<version>` with your installed GCC version (e.g., `15.2.0`). You can find it by running `g++ --version`.

When you compile a module interface unit (a `.cppm` or the `std.cc` file) with the `-fmodules-ts` flag, GCC produces two things:

1. **An object file** (`.o`) — contains the compiled code, needed at link time.
2. **A compiled module interface** (`.gcm`) — stored in a `gcm.cache/` directory in your build folder. This is what allows other files to `import` the module.

The build must respect the **dependency order**: a module must be compiled before any file that imports it.

## Building from the Command Line

All commands below should be run from the `with-gcc/` directory.

### Step 1: Compile the `std` Module

```powershell
g++ -std=c++23 -fmodules-ts -g -c "C:\msys64\ucrt64\include\c++\15.2.0\bits\std.cc"
```

This produces:
- `std.o` — the compiled object file for the standard library module.
- `gcm.cache\std.gcm` — the compiled module interface that allows `import std;` to work.

> **Important:** This step can take several seconds as it compiles the entire C++ standard library into a module. You only need to redo this if you change your GCC version or delete the `gcm.cache/` directory.

### Step 2: Compile Your Custom Module(s)

```powershell
g++ -std=c++23 -fmodules-ts -g -c mod.cppm
```

This produces:
- `mod.o` — the compiled object file for your custom module.
- `gcm.cache\mod.gcm` — the compiled module interface that allows `import mod;` to work.

> **Note:** If your custom module uses `import std;` internally, then Step 1 must be completed first.

### Step 3: Compile and Link the Main Program

```powershell
g++ -std=c++23 -fmodules-ts -g main.cpp std.o mod.o -o main.exe
```

This compiles `main.cpp` (which contains `import std;` and `import mod;`), links it against both module object files, and produces the final `main.exe`.

### Step 4: Run It

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
g++ -std=c++23 -fmodules-ts -g -c "C:\msys64\ucrt64\include\c++\15.2.0\bits\std.cc" && g++ -std=c++23 -fmodules-ts -g -c mod.cppm && g++ -std=c++23 -fmodules-ts -g main.cpp std.o mod.o -o main.exe
```

## Building from VS Code

The included `.vscode/tasks.json` defines three build tasks chained together with `dependsOn`, so you only need to trigger one:

1. Press **Ctrl+Shift+B** to run the default build task.
2. VS Code will automatically execute the tasks in order:
   - **Compile std module** → compiles `std.cc` into `std.o` + `gcm.cache/std.gcm`
   - **Compile project modules** → compiles `mod.cppm` into `mod.o` + `gcm.cache/mod.gcm`
   - **Build and link** → compiles `main.cpp` and links everything into `main.exe`

To debug, press **F5**. The `launch.json` is configured to build first (via the pre-launch task) and then launch `main.exe` under GDB.

## Clean Build

To do a clean rebuild, delete the generated artifacts and rebuild:

```powershell
Remove-Item -Recurse -Force gcm.cache, *.o, main.exe
```

Then follow the build steps above again.

## Key Compiler Flags

| Flag | Purpose |
|------|---------|
| `-std=c++23` | Enables the C++23 standard, which includes module support |
| `-fmodules-ts` | Enables GCC's C++20 modules implementation (the `-ts` refers to the original Technical Specification that was merged into C++20) |
| `-c` | Compile only (produce `.o` object file), do not link — required when compiling module interface units |
| `-g` | Include debug symbols for use with GDB |

## Troubleshooting

### `error: failed to read compiled module: No such file or directory`

This means a module's `.gcm` file is missing from `gcm.cache/`. Make sure you compile modules **in dependency order** — `std` first, then your custom modules, then the files that import them.

### `error: import std; module not found` or similar

Verify that the `std.cc` path matches your GCC version. Run `g++ --version` and update the path in the build commands accordingly.

### `cannot find -lstdc++: No such file or directory` or linker errors

Make sure you include `std.o` (and any other module `.o` files) on the link command line. Module object files must be explicitly linked — they are not pulled in automatically.

### IntelliSense errors in VS Code

The C/C++ extension's IntelliSense engine may not fully support C++ modules yet. If you see red squiggles on `import` statements but the code compiles fine, you can add this to your `.vscode/settings.json` to suppress them:

```json
{
    "C_Cpp.errorSquiggles": "disabled"
}
```