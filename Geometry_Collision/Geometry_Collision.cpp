// Geometry_Collision demo launcher.
//
// Opens one of the three demo windows. Each window is adapted from a GTE 6.x
// sample (David Eberly, Geometric Tools, Boost Software License 1.0), and
// main() follows the structure of the samples' *Main.cpp files: create the
// window through TheWindowSystem, run the message pump, destroy the window.
//
//   Geometry_Collision moving     moving sphere vs. aligned box (default),
//                                 sample Intersection/MovingSphereBox
//   Geometry_Collision boxsphere  oriented box vs. sphere test-intersection,
//                                 sample Intersection/IntersectBoxSphere
//   Geometry_Collision bounce     bouncing spheres physics,
//                                 sample Physics/BouncingSpheres

#include "BouncingSpheresWindow.h"
#include "BoxSphereIntersectionWindow.h"
#include "MovingSphereBoxWindow.h"
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

namespace
{
    template <typename WindowType>
    void Run(std::wstring const& title, int32_t xSize, int32_t ySize)
    {
        Window::Parameters parameters(title, 0, 0, xSize, ySize);
        auto window = TheWindowSystem.Create<WindowType>(parameters);
        TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
        TheWindowSystem.Destroy(window);
    }

    void PrintUsage(char const* program)
    {
        std::cout
            << "usage: " << program << " [moving | boxsphere | bounce]\n"
            << "  moving     moving sphere vs. aligned box (default)\n"
            << "  boxsphere  oriented box vs. sphere test-intersection\n"
            << "  bounce     bouncing spheres physics\n";
    }
}

int main(int argc, char* argv[])
{
    std::string const demo = (argc > 1 ? argv[1] : "moving");

    try
    {
        if (demo == "moving")
        {
            Run<MovingSphereBoxWindow3>(L"MovingSphereBoxWindow3", 768, 768);
        }
        else if (demo == "boxsphere")
        {
            Run<IntersectBoxSphereWindow3>(L"IntersectBoxSphereWindow3", 512, 512);
        }
        else if (demo == "bounce")
        {
            Run<BouncingSpheresWindow3>(L"BouncingSpheresWindow3", 1024, 768);
        }
        else
        {
            PrintUsage(argv[0]);
            return 1;
        }
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
