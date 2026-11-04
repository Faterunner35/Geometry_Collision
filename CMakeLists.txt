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

option(GC_BUILD_APP
    "Build the demo application with the vendored GTE Graphics/Applications (Windows: DirectX 11, Linux: OpenGL/GLX)"
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
#  gc_header_check: compiles every geometry header in its own translation unit
#  (included twice, to check the include guard). This catches headers that are
#  not self-contained.
# -----------------------------------------------------------------------------
set(GC_HEADERS
    AlignedBox.h
    CanonicalBox.h
    Constants.h
    DCPQuery.h
    DistPointAlignedBox.h
    DistPointCanonicalBox.h
    DistPointOrientedBox.h
    Exceptions.h
    FIQuery.h
    HyperPlane.h
    HyperSphere.h
    IntrAlignedBoxSphere.h
    IntrIntervals.h
    IntrLineAlignedBox.h
    IntrOrientedBoxShere.h
    IntrRayAlignedBox.h
    Line.h
    OrientedBox.h
    Ray.h
    RigidBody.h
    TIQuery.h
    Vector.h
    typeTraits_GM.h)

set(GC_HEADER_CHECK_SOURCES)
foreach(header IN LISTS GC_HEADERS)
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

add_executable(gc_geometry_tests tests/geometry_tests.cpp tests/test_harness.h)
target_link_libraries(gc_geometry_tests PRIVATE gc_geometry)
set_target_properties(gc_geometry_tests PROPERTIES FOLDER "Tests")
add_test(NAME geometry_tests COMMAND gc_geometry_tests)

# -----------------------------------------------------------------------------
#  Geometry_Collision: the demo application (three windows adapted from GTE
#  samples). It needs the vendored GTE Graphics and Applications libraries,
#  built here per platform:
#    Windows: DirectX 11 engine + Win32 windows (MSW)
#    Linux:   OpenGL 4.5 engine + X11 windows (GLX); needs libx11-dev,
#             libgl-dev, libegl-dev, libpng-dev
#  GTE code includes <Mathematics/...>, <Graphics/...>, <Applications/...>,
#  so the include root is Geometry_Collision/ itself; Mathematics/ is never
#  put on the include path (its Math.h would shadow <math.h> on Windows).
# -----------------------------------------------------------------------------
if(GC_BUILD_APP)
    set(GC_GTE_DEFS GTE_USE_ROW_MAJOR GTE_USE_MAT_VEC GTE_DISABLE_PCH)
    if(WIN32)
        list(APPEND GC_GTE_DEFS GTE_USE_MSWINDOWS GTE_USE_DIRECTX UNICODE _UNICODE)
        file(GLOB GC_GTE_GRAPHICS_SRC CONFIGURE_DEPENDS
            ${GC_SRC_DIR}/Graphics/*.cpp ${GC_SRC_DIR}/Graphics/DX11/*.cpp)
        file(GLOB GC_GTE_APP_SRC CONFIGURE_DEPENDS
            ${GC_SRC_DIR}/Applications/*.cpp ${GC_SRC_DIR}/Applications/MSW/*.cpp)
        set(GC_GTE_LIBS d3d11 d3dcompiler dxgi dxguid windowscodecs)
    elseif(UNIX AND NOT APPLE)
        list(APPEND GC_GTE_DEFS GTE_USE_LINUX GTE_USE_OPENGL)
        find_package(X11 REQUIRED)
        find_package(OpenGL REQUIRED COMPONENTS EGL)
        find_package(PNG REQUIRED)
        find_package(Threads REQUIRED)
        file(GLOB GC_GTE_GRAPHICS_SRC CONFIGURE_DEPENDS
            ${GC_SRC_DIR}/Graphics/*.cpp
            ${GC_SRC_DIR}/Graphics/GL45/*.cpp
            ${GC_SRC_DIR}/Graphics/GL45/GLX/*.cpp)
        file(GLOB GC_GTE_APP_SRC CONFIGURE_DEPENDS
            ${GC_SRC_DIR}/Applications/*.cpp ${GC_SRC_DIR}/Applications/GLX/*.cpp)
        set(GC_GTE_LIBS X11::X11 OpenGL::GL OpenGL::EGL PNG::PNG Threads::Threads)
    else()
        message(FATAL_ERROR "GC_BUILD_APP supports Windows (DirectX 11) and Linux (OpenGL/GLX).")
    endif()

    add_library(gc_gte_graphics STATIC ${GC_GTE_GRAPHICS_SRC})
    add_library(gc_gte_applications STATIC ${GC_GTE_APP_SRC})
    foreach(t gc_gte_graphics gc_gte_applications)
        target_include_directories(${t} PUBLIC ${GC_SRC_DIR})
        target_compile_definitions(${t} PUBLIC ${GC_GTE_DEFS})
        set_target_properties(${t} PROPERTIES FOLDER "GTE")
        if(MSVC)
            target_compile_options(${t} PRIVATE /utf-8 /MP /W1)
            target_compile_definitions(${t} PUBLIC NOMINMAX)
        endif()
    endforeach()
    target_link_libraries(gc_gte_applications PUBLIC gc_gte_graphics)
    target_link_libraries(gc_gte_graphics PUBLIC ${GC_GTE_LIBS})

    add_executable(Geometry_Collision
        ${GC_SRC_DIR}/BouncingSpheresWindow.cpp
        ${GC_SRC_DIR}/BoxSphereIntersectionWindow.cpp
        ${GC_SRC_DIR}/Geometry_Collision.cpp
        ${GC_SRC_DIR}/MovingSphereBoxWindow.cpp
        ${GC_SRC_DIR}/PhysModule.cpp
        ${GC_SRC_DIR}/RigidPlane.cpp
        ${GC_SRC_DIR}/RigidSphere.cpp)
    target_link_libraries(Geometry_Collision PRIVATE gc_geometry gc_gte_applications)
    # BouncingSpheres loads Data/BallTextureWrap.png and Data/Initial.txt. It
    # searches GC_DATA_DIR (the source folder) and Data/ next to the working
    # directory; the folder is also copied next to the executable.
    target_compile_definitions(Geometry_Collision PRIVATE GC_DATA_DIR="${GC_SRC_DIR}/Data")
    add_custom_command(TARGET Geometry_Collision POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${GC_SRC_DIR}/Data $<TARGET_FILE_DIR:Geometry_Collision>/Data)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Geometry_Collision)
else()
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT gc_geometry_tests)
endif()
