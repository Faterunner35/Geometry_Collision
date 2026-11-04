cmake_minimum_required(VERSION 3.20)
project(Geometry_Collision LANGUAGES CXX)

# The Visual Studio project (Geometry_Collision.vcxproj) uses C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(GC_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Geometry_Collision)

option(GC_CHECK_ALL_HEADERS
    "Also compile the geometry headers that are known not to compile yet (see README, 'Known issues')"
    OFF)
option(GC_BUILD_APP
    "Windows only: build the demo/physics sources exactly as Geometry_Collision.vcxproj does. Does NOT compile yet (see README)"
    OFF)

# -----------------------------------------------------------------------------
#  Common compiler settings
# -----------------------------------------------------------------------------
add_library(gc_options INTERFACE)
if(MSVC)
    target_compile_options(gc_options INTERFACE /utf-8 /W3 /permissive- /Zc:__cplusplus /MP)
    target_compile_definitions(gc_options INTERFACE _USE_MATH_DEFINES NOMINMAX)
else()
    target_compile_options(gc_options INTERFACE -Wall -Wextra -Wno-unused-parameter)
endif()

# -----------------------------------------------------------------------------
#  gc_geometry: header-only geometry code in Geometry_Collision/.
#
#  Only Geometry_Collision/ itself is put on the include path. Mathematics/,
#  Graphics/ and Applications/ are deliberately NOT added: Mathematics/Math.h
#  would shadow the system <math.h> on case-insensitive file systems (Windows),
#  which is exactly what breaks the Visual Studio build (see README).
# -----------------------------------------------------------------------------
add_library(gc_geometry INTERFACE)
target_include_directories(gc_geometry INTERFACE ${GC_SRC_DIR})
target_link_libraries(gc_geometry INTERFACE gc_options)

# -----------------------------------------------------------------------------
#  gc_header_check: compiles every listed header in its own translation unit
#  (included twice, to check the include guard). This catches headers that are
#  not self-contained.
#
#  GC_HEADERS_OK      compile today with GCC and MSVC.
#  GC_HEADERS_PENDING the geometry/intersection headers; they do not compile yet
#                     because of the unfinished gte/gtl -> Vector_GM migration.
#                     Move a header to GC_HEADERS_OK once it compiles.
# -----------------------------------------------------------------------------
set(GC_HEADERS_OK
    Constants.h
    DCPQuery.h
    Exceptions.h
    FIQuery.h
    TIQuery.h
    typeTraits_GM.h)

set(GC_HEADERS_PENDING
    AlignedBox.h
    CanonicalBox.h
    DistPointAlignedBox.h
    DistPointCanonicalBox.h
    DistPointOrientedBox.h
    HyperPlane.h
    HyperSphere.h
    InterRayAlignedBox.h
    IntrAlignedBoxSphere.h
    IntrIntervals.h
    IntrLineAlignedBox.h
    IntrOrientedBoxShere.h
    IntrRayAlignedBox.h
    Line.h
    OrientedBox.h
    QueryBoundedBox.h
    Ray.h
    Vector.h)

set(GC_HEADERS_TO_CHECK ${GC_HEADERS_OK})
if(GC_CHECK_ALL_HEADERS)
    list(APPEND GC_HEADERS_TO_CHECK ${GC_HEADERS_PENDING})
endif()

set(GC_HEADER_CHECK_SOURCES)
foreach(header IN LISTS GC_HEADERS_TO_CHECK)
    string(MAKE_C_IDENTIFIER "${header}" header_id)
    set(tu ${CMAKE_CURRENT_BINARY_DIR}/header_check/${header_id}.cpp)
    set(GC_HEADER "${header}")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/header_check.cpp.in ${tu} @ONLY)
    list(APPEND GC_HEADER_CHECK_SOURCES ${tu})
endforeach()

add_library(gc_header_check OBJECT ${GC_HEADER_CHECK_SOURCES})
target_link_libraries(gc_header_check PRIVATE gc_geometry)
set_target_properties(gc_header_check PROPERTIES FOLDER "Checks")

# -----------------------------------------------------------------------------
#  Tests
# -----------------------------------------------------------------------------
enable_testing()

add_executable(gc_smoke_tests tests/smoke_tests.cpp)
target_link_libraries(gc_smoke_tests PRIVATE gc_geometry)
set_target_properties(gc_smoke_tests PROPERTIES FOLDER "Tests")
add_test(NAME smoke_tests COMMAND gc_smoke_tests)

# -----------------------------------------------------------------------------
#  Geometry_Collision: the Visual Studio application, mirrored 1:1 from
#  Geometry_Collision.vcxproj (same sources, same include directories).
#  Off by default because it does not compile yet; see README.
# -----------------------------------------------------------------------------
if(GC_BUILD_APP)
    if(NOT WIN32)
        message(FATAL_ERROR "GC_BUILD_APP requires Windows (GTE DirectX 11 / Win32 window code).")
    endif()
    message(WARNING "GC_BUILD_APP=ON: these sources are known not to compile yet (see README, 'Known issues').")

    set(GC_APP_SOURCES
        ${GC_SRC_DIR}/BouncingSpheresWindow.cpp
        ${GC_SRC_DIR}/BoxSphereIntersectionWindow.cpp
        ${GC_SRC_DIR}/Geometry_Collision.cpp
        ${GC_SRC_DIR}/MovingSphereBoxWindow.cpp
        ${GC_SRC_DIR}/PhysModule.cpp
        ${GC_SRC_DIR}/RigidPlane.cpp
        ${GC_SRC_DIR}/RigidSphere.cpp)

    add_executable(Geometry_Collision ${GC_APP_SOURCES})
    target_include_directories(Geometry_Collision PRIVATE
        ${GC_SRC_DIR}
        ${GC_SRC_DIR}/Mathematics
        ${GC_SRC_DIR}/Graphics
        ${GC_SRC_DIR}/Applications)
    target_compile_definitions(Geometry_Collision PRIVATE _CONSOLE)
    target_link_libraries(Geometry_Collision PRIVATE gc_options)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Geometry_Collision)
else()
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT gc_smoke_tests)
endif()
