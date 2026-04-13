# gcc-c

A sample project demonstrating how to compile C code with GCC via [MSYS2](https://www.msys2.org/) UCRT64, using CMake and Ninja, with full Visual Studio integration.

## Table of Contents

- [Overview](#overview)
- [Prerequisites](#prerequisites)
  - [Visual Studio](#visual-studio)
  - [MSYS2](#msys2)
- [Project Structure](#project-structure)
- [How It Works](#how-it-works)
  - [CMakeLists.txt](#cmakeliststxt)
  - [CMakePresets.json](#cmakepresetsjson)
- [Building the Project](#building-the-project)
  - [Building from Visual Studio](#building-from-visual-studio)
  - [Building from the Command Line](#building-from-the-command-line)
- [Running the Executable](#running-the-executable)
- [Troubleshooting](#troubleshooting)

## Overview

This project is a minimal example of a C program compiled with GCC on Windows. Instead of using the Microsoft Visual C++ (MSVC) compiler that Visual Studio uses by default, it targets the GCC compiler provided by MSYS2's UCRT64 environment. This is useful when you need GCC-specific features, want to match a Linux GCC build environment, or simply prefer GCC over MSVC.

The key technologies involved are:

- **GCC (GNU Compiler Collection):** The C compiler used to build the source code.
- **MSYS2 UCRT64:** A Windows software distribution that provides GCC and other Unix tools. The UCRT64 environment specifically targets the Universal C Runtime (UCRT), which is the modern C runtime included in Windows 10 and later.
- **CMake:** A cross-platform build system generator. It reads `CMakeLists.txt` to produce build files for a chosen backend (in our case, Ninja).
- **Ninja:** A small, fast build system. CMake generates `build.ninja` files, and Ninja executes the actual compilation and linking. Visual Studio ships with a bundled copy of Ninja.
- **Visual Studio (Open Folder mode):** Visual Studio can open a folder containing a `CMakePresets.json` (or `CMakeSettings.json`) and automatically configure, build, and debug CMake-based projects without requiring a `.sln` or `.vcxproj` file.

## Prerequisites

### Visual Studio

You need **Visual Studio 2019 or later** (the Community edition is free) with the following workloads and components installed:

1. Open the **Visual Studio Installer**.
2. Click **Modify** on your Visual Studio installation.
3. Under the **Workloads** tab, ensure the following is checked:
   - **Desktop development with C++**
4. Under the **Individual components** tab (use the search box), ensure the following are checked:
   - **C++ CMake tools for Windows** — this installs both CMake and Ninja as bundled tools inside Visual Studio.

These components give Visual Studio the ability to open CMake-based folder projects and build them using the bundled CMake and Ninja executables.

> **Note:** You do _not_ need to install CMake or Ninja separately. Visual Studio includes its own copies. However, if you want to build from the command line outside of Visual Studio's Developer Command Prompt, you will need CMake and Ninja on your system `PATH`. You can install them via MSYS2 (see below), [Scoop](https://scoop.sh/), [Chocolatey](https://chocolatey.org/), or from their official websites.

### MSYS2

[MSYS2](https://www.msys2.org/) provides the GCC compiler and associated toolchain. Follow these steps to install and configure it:

1. **Download and install MSYS2** from [https://www.msys2.org/](https://www.msys2.org/). The default installation path is `C:\msys64`. The rest of this document assumes this default path.

2. **Open the MSYS2 UCRT64 terminal.** After installation, MSYS2 provides several terminal shortcuts in the Start Menu. Open the one labeled **MSYS2 UCRT64** (it has a teal/cyan-coloured icon). It is important to use the UCRT64 terminal specifically, as each terminal targets a different environment and toolchain.

3. **Update the package database and core packages:**

   ```bash
   pacman -Syu
   ```

   The terminal may close and ask you to reopen it. If so, reopen the **MSYS2 UCRT64** terminal and run:

   ```bash
   pacman -Su
   ```

4. **Install the UCRT64 GCC toolchain:**

   ```bash
   pacman -S mingw-w64-ucrt-x86_64-toolchain
   ```

   When prompted to select members, press **Enter** to install all members of the group. This installs GCC, G++, GDB (the GNU debugger), and related tools.

5. **Verify the installation.** Still in the MSYS2 UCRT64 terminal, run:

   ```bash
   gcc --version
   ```

   You should see output indicating the GCC version, such as `gcc (Rev1, Built by MSYS2 project) 15.x.x`.

6. **(Optional but recommended) Add MSYS2 UCRT64 to your Windows system `PATH`.**

   Add the following directory to your Windows `PATH` environment variable:

   ```
   C:\msys64\ucrt64\bin
   ```

   This allows tools like `gcc`, `g++`, and `gdb` to be found from any terminal (PowerShell, CMD, Visual Studio Developer Command Prompt, etc.). To do this:

   - Press **Win + R**, type `sysdm.cpl`, and press Enter.
   - Go to the **Advanced** tab and click **Environment Variables**.
   - Under **User variables** (or **System variables** if you want it for all users), select `Path` and click **Edit**.
   - Click **New** and add `C:\msys64\ucrt64\bin`.
   - Click **OK** on all dialogs.
   - Restart any open terminals for the change to take effect.

   > **Why UCRT64 and not MINGW64?** MSYS2 offers several environments. UCRT64 uses the Universal C Runtime (UCRT), which is the same runtime that MSVC uses on modern Windows. This means better compatibility with Windows system libraries and other MSVC-compiled code. MINGW64, on the other hand, uses the older MSVCRT runtime, which has known limitations (e.g., incomplete C99 `printf` format support). For new projects, UCRT64 is the recommended choice.

## Project Structure

```
gcc-c/
├── CMakeLists.txt          # CMake build configuration
├── CMakePresets.json        # CMake presets for MSYS2 UCRT64 (Debug & Release)
├── main.c                   # C source file
└── README.md                # This file
```

## How It Works

### CMakeLists.txt

The `CMakeLists.txt` file is the CMake project definition:

```cmake
cmake_minimum_required(VERSION 3.20)
project(gcc-c LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.c)
```

- `cmake_minimum_required(VERSION 3.20)` — requires CMake 3.20 or later. This version introduced full support for CMake Presets (`CMakePresets.json`).
- `project(gcc-c LANGUAGES C)` — declares a project named `gcc-c` that uses the C language. Specifying `LANGUAGES C` tells CMake to only look for a C compiler (not C++).
- `CMAKE_C_STANDARD 17` — targets the C17 standard (ISO/IEC 9899:2018).
- `CMAKE_C_STANDARD_REQUIRED ON` — ensures the build fails if the compiler does not support C17, rather than silently falling back to an older standard.
- `add_executable(${PROJECT_NAME} main.c)` — builds an executable named `gcc-c` from `main.c`.

### CMakePresets.json

The `CMakePresets.json` file defines named configuration and build presets. This is what allows Visual Studio (and command-line CMake) to know _how_ to configure the project:

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "ucrt64-debug",
            "displayName": "MSYS2 UCRT64 Debug",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "C:/msys64/ucrt64/bin/gcc.exe",
                "CMAKE_BUILD_TYPE": "Debug"
            }
        },
        {
            "name": "ucrt64-release",
            "displayName": "MSYS2 UCRT64 Release",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "C:/msys64/ucrt64/bin/gcc.exe",
                "CMAKE_BUILD_TYPE": "Release"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "ucrt64-debug",
            "configurePreset": "ucrt64-debug"
        },
        {
            "name": "ucrt64-release",
            "configurePreset": "ucrt64-release"
        }
    ]
}
```

Key points:

- **`generator: "Ninja"`** — uses Ninja as the build backend. Ninja is fast and works well with GCC. Visual Studio bundles its own copy of Ninja, so no separate installation is needed when building from within VS.
- **`CMAKE_C_COMPILER`** — points directly to the MSYS2 UCRT64 GCC executable. This is what tells CMake to use GCC instead of MSVC. If your MSYS2 installation is in a different location, update this path accordingly.
- **`CMAKE_BUILD_TYPE`** — set to `Debug` or `Release`. In Debug mode, GCC includes debug symbols (`-g`) and disables optimisations. In Release mode, GCC enables optimisations (`-O2` or `-O3` depending on CMake's defaults).
- **`binaryDir`** — build output goes to `build/ucrt64-debug/` or `build/ucrt64-release/` under the project directory, keeping source and build files separate.

## Building the Project

### Building from Visual Studio

1. **Open the project folder** in Visual Studio:
   - Go to **File → Open → Folder...** and select the `gcc-c` directory.
   - Visual Studio will detect the `CMakePresets.json` and begin configuring automatically.

2. **Select a configuration preset** from the toolbar dropdown. You should see:
   - **MSYS2 UCRT64 Debug**
   - **MSYS2 UCRT64 Release**

   Select the one you want.

3. **Wait for CMake configuration to complete.** The Output window (View → Output, show output from **CMake**) will show progress. You should see messages like:
   ```
   -- The C compiler identification is GNU 15.x.x
   -- Configuring done
   -- Generating done
   ```

4. **Build the project:**
   - Go to **Build → Build All** (or press **Ctrl+Shift+B**).
   - The Output window will show Ninja compiling `main.c` and linking `gcc-c.exe`.

5. **Run the executable:**
   - The built executable is located at `build/ucrt64-debug/gcc-c.exe` (or `build/ucrt64-release/gcc-c.exe`).
   - You can run it from the **Debug → Start Without Debugging** menu (Ctrl+F5), or directly from a terminal.

### Building from the Command Line

If you prefer to build outside of Visual Studio, you can use CMake directly from PowerShell or CMD. You need `cmake`, `ninja`, and `gcc` on your `PATH`.

If you have Visual Studio's CMake and Ninja on your `PATH` (e.g., via the **Developer PowerShell for VS**), and MSYS2 UCRT64's `bin` directory on your `PATH`:

1. **Open a terminal** and navigate to the project directory:

   ```powershell
   cd path\to\gcc-c
   ```

2. **Configure the project** using a preset:

   ```powershell
   cmake --preset ucrt64-debug
   ```

   This creates the `build/ucrt64-debug/` directory and generates Ninja build files.

3. **Build the project:**

   ```powershell
   cmake --build --preset ucrt64-debug
   ```

   Ninja will compile and link the project.

4. **Run the executable:**

   ```powershell
   .\build\ucrt64-debug\gcc-c.exe
   ```

To build the Release version instead, replace `ucrt64-debug` with `ucrt64-release` in the commands above.

## Running the Executable

After a successful build, running `gcc-c.exe` produces output similar to:

```
Hello from GCC (MSYS2 UCRT64)!
Compiler: 15.2.0
C Standard: 201710
```

- **Compiler** — the GCC version string.
- **C Standard** — `201710` corresponds to C17 (the `__STDC_VERSION__` macro value defined by the C17 standard).

## Troubleshooting

### CMake cannot find the compiler

```
-- The C compiler identification is unknown
CMake Error: CMAKE_C_COMPILER not set
```

This means CMake cannot find `gcc.exe` at the path specified in `CMakePresets.json`. Verify that:

- MSYS2 is installed at `C:\msys64` (or update the path in `CMakePresets.json`).
- The UCRT64 GCC toolchain is installed (`pacman -S mingw-w64-ucrt-x86_64-toolchain` in the MSYS2 UCRT64 terminal).
- The file `C:\msys64\ucrt64\bin\gcc.exe` exists.

### Ninja is not found

```
CMake Error: CMake was unable to find a build program corresponding to "Ninja".
```

If building from the command line, ensure Ninja is on your `PATH`. Options:

- Use the **Developer PowerShell for VS** (which adds VS-bundled Ninja to the `PATH`).
- Install Ninja via MSYS2: `pacman -S mingw-w64-ucrt-x86_64-ninja`
- Install Ninja via [Scoop](https://scoop.sh/): `scoop install ninja`

### IntelliSense shows red squiggles in Visual Studio

Visual Studio's IntelliSense may not automatically resolve headers from the MSYS2 GCC include paths. If you see false errors on standard headers like `<stdio.h>`:

- Wait for the CMake configuration to finish. IntelliSense updates after CMake completes.
- Ensure the correct preset is selected in the toolbar dropdown.
- Try **Project → Delete Cache and Reconfigure** to force a fresh CMake configuration.
- Check that `compile_commands.json` is being generated in the build directory (CMake with Ninja does this by default). IntelliSense uses this file for accurate include paths.

### Executable fails to run or crashes

If the executable fails to start with a missing DLL error (e.g., `libgcc_s_seh-1.dll`), it means the MSYS2 UCRT64 runtime DLLs are not on the `PATH`. Either:

- Add `C:\msys64\ucrt64\bin` to your system `PATH` (recommended — see [Prerequisites](#msys2)).
- Copy the required DLLs from `C:\msys64\ucrt64\bin` into the same directory as the executable.
- Link statically by adding `-static` to the linker flags in `CMakeLists.txt`:
  ```cmake
  target_link_options(${PROJECT_NAME} PRIVATE -static)
  ```
