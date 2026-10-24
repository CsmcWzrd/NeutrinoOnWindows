# Split verification — 25 October 2026

Baseline: Neutrino_Stage2_23_Tabbar_Positions_Fixed.zip (retrieved from the existing Neutrino Project files).

Scope: independent package and build-system separation. Platform conditionals
were resolved to their existing branches. Public API names, signatures, include
path, namespace, implementation logic and visual assets are preserved. Original
license notice and release history are included. Existing platform differences
and pre-existing limitations are retained.

Checks for Neutrino:

- Compiler-preprocessed baseline/split comparison passed for all 32
  implementation and example translation units, including their public header.
  Nonempty preprocessed lines are identical; blank lines are ignored.
- Every copied theme/icon asset is byte-for-byte identical to the baseline.
- Every source/header checksum matches SOURCE_PROVENANCE.json.
- Windows x64 MinGW-w64 CMake cross-build: static library and all 30 demos
  compiled and linked successfully, including the original native Win32 demo.
- CMake install plus an independent find_package(Neutrino) consumer cross-built
  successfully.
- All 31 VS2022 vcxproj files parsed successfully; all source/header/project
  references resolve within this package. Every project and the solution include
  Debug/Release for Win32, x64 and ARM64, using v143 and Windows SDK 10.0.
- Actual MSVC/NMake execution and native Windows runtime tests were not possible
  on this Linux host. Win32/ARM64 configurations have not been compiled here.
  Windows 10+ desktop targeting is configured; testing every OS edition or
  architecture is not claimed.
