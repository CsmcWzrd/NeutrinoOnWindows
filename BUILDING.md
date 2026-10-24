# Building Neutrino (Windows 10+)

Windows desktop editions of Windows 10 and later, including Windows 11; this is
a Win32 desktop library, not a UWP library. OS edition does not select a different
API. Windows SDK targeting is WINVER/_WIN32_WINNT=0x0A00. Native validation on each
Windows edition is still required; no Windows runtime execution was performed here.

Install Visual Studio 2022 Desktop development with C++, v143 and a Windows 10/11
SDK. x86 uses the Win32 configuration; x64 and ARM64 configurations are included.
ARM64 requires its corresponding VS2022 C++ toolchain component.

Open `msvc/Neutrino.sln`, select Debug/Release and Win32/x64/ARM64, then Build Solution.
Every original demo has its own project and a reference to the static library.
Outputs: `msvc/bin/<platform>/<configuration>/`; object folders are per project
and architecture. Individual .vcxproj builds also use project-relative paths.

CMake with VS2022:

```bat
cmake -S . -B build-vs2022 -G "Visual Studio 17 2022" -A x64
cmake --build build-vs2022 --config Release
cmake --install build-vs2022 --config Release --prefix C:/Neutrino
```

Use `-A Win32` or `-A ARM64` in a separate build directory for those architectures.
CMake also supports NMake Makefiles or Ninja from a VS2022 Developer Command Prompt.

NMake from the Developer Command Prompt for your target architecture:

```bat
nmake /f Makefile.msvc CONFIG=Release
nmake /f Makefile.msvc CONFIG=Debug
```

Use a separate BUILD_DIR per architecture. `Makefile` and `Makefile.windows`
are aliases. Output: `build-msvc/<configuration>/`.

MinGW-w64 (GNU make running under Windows cmd.exe):

```bat
mingw32-make -f Makefile.mingw
```

Alternatively use CMake's MinGW Makefiles generator. All build routes compile
only `src/win32/Neu_Win32.cpp` and `src/Neu_Cards.cpp`; no X11/Unix dependency.
CMake installs the headers, library, demos, assets, and `NeutrinoConfig.cmake`.
Downstream CMake: `find_package(Neutrino CONFIG REQUIRED)` then link
`Neutrino::Neutrino`. Set `-DNEUTRINO_BUILD_EXAMPLES=OFF` for library-only builds.

Run demos with this package root as the working directory for the original
relative asset paths. With installed demos, use the installed share/Neutrino
folder as working directory. No rendering, event, asset lookup or API changes
were made by this split.
