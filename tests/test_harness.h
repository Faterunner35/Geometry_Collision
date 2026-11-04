// Minimal test harness for the Geometry_Collision tests: no dependencies, no
// network. Each CHECK prints one line; main() returns the failure count
// through gc_test::Summary(), so CTest sees a non-zero exit code on failure.

#pragma once

#include <cmath>
#include <cstdio>
#include <string>

namespace gc_test
{
    inline int& Failures()
    {
        static int failures = 0;
        return failures;
    }

    inline int& Checks()
    {
        static int checks = 0;
        return checks;
    }

    inline void Report(bool ok, char const* file, int line, std::string const& what)
    {
        ++Checks();
        if (!ok)
        {
            ++Failures();
            std::printf("[FAIL] %s:%d: %s\n", file, line, what.c_str());
        }
    }

    inline bool Near(double a, double b, double tolerance)
    {
        return std::fabs(a - b) <= tolerance;
    }

    inline void Section(char const* name)
    {
        std::printf("-- %s\n", name);
    }

    inline int Summary()
    {
        std::printf("%d checks, %d failed\n", Checks(), Failures());
        return Failures() == 0 ? 0 : 1;
    }
}

#define GC_CHECK(condition) \
    gc_test::Report((condition), __FILE__, __LINE__, #condition)

#define GC_CHECK_NEAR(actual, expected, tolerance) \
    gc_test::Report(gc_test::Near((actual), (expected), (tolerance)), __FILE__, __LINE__, \
        std::string(#actual " == " #expected ": got ") + std::to_string(actual) + \
        ", expected " + std::to_string(expected))

// Componentwise comparison of two 3D vectors.
#define GC_CHECK_VEC3(actual, x, y, z, tolerance) \
    do { \
        GC_CHECK_NEAR((actual)[0], (x), (tolerance)); \
        GC_CHECK_NEAR((actual)[1], (y), (tolerance)); \
        GC_CHECK_NEAR((actual)[2], (z), (tolerance)); \
    } while (false)
