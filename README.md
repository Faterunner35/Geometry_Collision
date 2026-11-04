# Geometry_Collision

A Visual Studio project for studying collision and distance queries between simple 3D primitives: point to box distance, line/ray to box, sphere to axis-aligned or oriented box (static and moving), and impulse-based rigid-sphere physics. The code is based on David Eberly's [Geometric Tools](https://www.geometrictools.com) (GTE and GTL) and is being ported into a single namespace, `Vector_GM`.

> **Status: work in progress, does not compile yet.** The port to `Vector_GM` is unfinished. GTE, GTL and the new namespace are mixed, so the geometry headers and the demo application fail to build with MSVC and with GCC (see [Known issues](#known-issues)). The CMake build and CI cover only the parts that compile today.

---

## Origin of the code

Almost all of the source here is Geometric Tools code by David Eberly, used under the [Boost Software License 1.0](licenses/BSL-1.0.txt). [THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt) has the file-by-file list.

| Location | Origin | Changes made in this repository |
|---|---|---|
| `Geometry_Collision/Mathematics/`, `Graphics/`, `Applications/` | GTE 6.x, vendored (file versions dated 2022 to 2023) | About 44 files partly renamed from `gte` to `Vector_GM`; about 30 of them lost their copyright header |
| `Geometry_Collision/*.h` geometry headers: `AlignedBox.h`, `OrientedBox.h`, `CanonicalBox.h`, `Line.h`, `Ray.h`, `HyperPlane.h`, `DistPoint*.h`, `Intr*.h`, `InterRayAlignedBox.h`, `QueryBoundedBox.h`, `DCPQuery.h`, `FIQuery.h`, `TIQuery.h`, `IntrIntervals.h`, `RigidBody.h` | Copies of GTE `Mathematics/` headers | Copyright header removed, includes switched to `"..."`, namespace `gte` changed to `Vector_GM` (not everywhere) |
| `Vector.h`, `Constants.h`, `Exceptions.h`, `typeTraits_GM.h`, `HyperSphere.h` | GTL (Geometric Tools Library: namespace `gtl`, `GTL_*` macros) | Partly renamed to `Vector_GM`; `GTL_ASSERT` renamed to `GM_ASSERT` |
| `BouncingSpheresWindow.*`, `PhysModule.*`, `RigidPlane.*`, `Rigidsphere.h`, `RigidSphere.cpp`, `BoxSphereIntersectionWindow.*`, `MovingSphereBoxWindow.*` | Adapted from the GTE samples *BouncingSpheres*, *IntersectBoxSphere* and *MovingSphereBox* (the class names, comments and structure match) | Include paths and namespace changed |
| `Geometry_Collision.cpp` | Visual Studio console template (`main` prints "Hello World!") | Not wired to the demo windows yet |
| `Geometry_Collision.sln`, `.vcxproj`, `.vcxproj.filters` | Project files | - |
| `CMakeLists.txt`, `CMakePresets.json`, `cmake/`, `tests/`, `.github/` | Build scaffolding added during the repository cleanup | - |

## Repository layout

```
Geometry_Collision.sln              Visual Studio 2022 solution
CMakeLists.txt, CMakePresets.json   CMake build (see below)
cmake/header_check.cpp.in           template for the header self-containment check
tests/smoke_tests.cpp               tests for the headers that compile today
Geometry_Collision/
  *.h, *.cpp                        geometry queries, physics and demo windows being ported
  Mathematics/                      GTE mathematics (header-only)
  Graphics/                         GTE graphics engine (DX11/, GL45/)
  Applications/                     GTE application framework (MSW/, GLX/)
licenses/BSL-1.0.txt                Boost Software License 1.0
THIRD_PARTY_NOTICES.txt             attribution, file by file
```

## Building

### Visual Studio

Open `Geometry_Collision.sln` in Visual Studio 2022 (toolset v143) and select **Debug | x64**. That is the only configuration with include directories (`Mathematics`, `Graphics`, `Applications`) and C++17 set. The build currently fails; see [Known issues](#known-issues).

### CMake

Requires CMake 3.20 or newer and a C++17 compiler (MSVC 2022, GCC 11 or newer, Clang 14 or newer).

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Presets: `x64-debug`, `x64-release` and `vs2022` on Windows; `linux-debug` and `linux-release` elsewhere. For example: `cmake --preset linux-release && cmake --build --preset linux-release`.

| Target | What it is |
|---|---|
| `gc_geometry` | Header-only INTERFACE target. Only `Geometry_Collision/` is on the include path, so `Mathematics/Math.h` cannot shadow `<math.h>`. |
| `gc_header_check` | Compiles each header in its own translation unit, included twice. `GC_HEADERS_OK` in `CMakeLists.txt` lists the headers that compile today (`Constants.h`, `DCPQuery.h`, `Exceptions.h`, `FIQuery.h`, `TIQuery.h`, `typeTraits_GM.h`). |
| `gc_smoke_tests` | CTest `smoke_tests`: covers `Constants.h`, `typeTraits_GM.h` and the `Exceptions.h` assertion macros. |
| `Geometry_Collision` | Only built with `-DGC_BUILD_APP=ON` (Windows). Uses the same sources and include directories as the `.vcxproj`. Does not compile yet. |

| Option | Default | Effect |
|---|---|---|
| `GC_CHECK_ALL_HEADERS` | `OFF` | Also checks the 18 headers in `GC_HEADERS_PENDING` (all geometry and intersection headers). Use it to track the port; move a header to `GC_HEADERS_OK` once it compiles. |
| `GC_BUILD_APP` | `OFF` | Builds the Windows demo application described above. |

CI (`.github/workflows/build.yml`) runs configure, build and test with MSVC 2022 on Windows and GCC on Linux.

## Known issues

These are the reasons the project does not build. Together they are the to-do list for finishing the port.

1. **Two different `Vector` templates.** The root `Vector.h` is GTL's `Vector<T, N>` (type first). Every box, line and ray header expects GTE's `Vector<N, T>` (dimension first), and `HyperSphere.h` is GTL-style while its users are GTE-style. `Vector.h` is also only partly renamed: some blocks are still in `namespace gtl`, and `C_` is not visible from `Vector_GM`.
2. **Mixed namespaces.** `DCPQuery.h`, `TIQuery.h` and `IntrAlignedBoxSphere.h` are still in `namespace gte`, but the specialisations that use them are in `Vector_GM`. The vendored `Mathematics/` and `Graphics/` folders are also half renamed: for example, `Mathematics/Vector.h` is in `Vector_GM`, while `Mathematics/CanonicalBox.h` is still in `gte`. `GaussianElimination.h` (in `Vector_GM`) uses `LexicoArray2` from `gte`.
3. **`TIQuery.h` defines `FIQuery`.** Its content is GTE's `FIQuery.h`, so the primary template `TIQuery` is never declared ("'TIQuery' is not a class template").
4. **`BouncingSpheresWindow.cpp` is a copy of `BouncingSpheresWindow.h`.** It starts with `#pragma once` and has no member definitions, so `BouncingSpheresWindow3` has no implementation.
5. **Duplicate specialisations.** `InterRayAlignedBox.h` and `IntrRayAlignedBox.h` both define `TIQuery`/`FIQuery<T, Ray3<T>, AlignedBox3<T>>`. `QueryBoundedBox.h` and `DistPointCanonicalBox.h` both define `DCPQuery<T, Vector<N,T>, CanonicalBox<N,T>>`. Root `IntrOrientedBoxShere.h` duplicates `Mathematics/IntrOrientedBox3Sphere3.h`. Including both files of any pair gives a redefinition error.
6. **Includes that do not resolve.** `BoxSphereIntersectionWindow.h` includes `IntrOrientedBox3Sphere3.h` (only in `Mathematics/`), not the root `IntrOrientedBoxShere.h`. `RigidSphere.cpp` includes `RigidSphere.h`, but the file is `Rigidsphere.h`. `Rigidsphere.h` and `RigidPlane.h` include `Hypersphere.h` and `Hyperplane.h`, but the files are `HyperSphere.h` and `HyperPlane.h`. These names work only on case-insensitive file systems. `HyperPlane.h`, `RigidBody.h`, `MovingSphereBoxWindow.cpp` and `BouncingSpheresWindow.*` use bare includes (`Matrix.h`, `Window3.h`, `MeshFactory.h`) that resolve only when `Mathematics/`, `Graphics/` and `Applications/` are on the include path.
7. **`<math.h>` is shadowed on Windows.** The `.vcxproj` puts `Mathematics/` on the include path. On a case-insensitive file system, GTE's `Mathematics/Math.h` is then found for every `#include <math.h>`, which produces the errors `'atan': is not a member of 'std'` and similar from the last committed MSBuild log. GTE expects its parent folder on the include path, with includes written as `<Mathematics/...>`.
8. **The Debug|x64 configuration prepends Visual Studio 2015 headers.** `IncludePath` is set to `C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include`, which mixes VS 2015 standard library headers with the v143 (VS 2022) compiler.
9. **GTE `.cpp` files are not in the project.** `Graphics/` and `Applications/` contain about 230 `.cpp` files, and the demo windows need many of them, but none are compiled or linked. Even with clean headers, the application would fail at link time.
10. **No entry point for the demos.** `main()` in `Geometry_Collision.cpp` prints "Hello World!" and never creates any of the three windows.
11. **`PhysModule.h`, `RigidPlane.h` and `Rigidsphere.h` have no include guard.**
12. **Rigid-sphere inertia.** `RigidSphere.cpp` sets the body inertia tensor to `massDensity * I`. The inertia of a solid sphere is `(2/5)·m·r²·I`. This may be inherited from the GTE sample; check against upstream before changing it.

## License

The Geometric Tools code (almost all of this repository) is licensed under the Boost Software License 1.0 ([licenses/BSL-1.0.txt](licenses/BSL-1.0.txt)), copyright David Eberly, Geometric Tools. See [THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt). The OpenGL headers in `Graphics/GL45/GL/` are copyright The Khronos Group and carry their own license. No separate license has been chosen yet for the repository owner's own additions.
