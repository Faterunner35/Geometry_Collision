// Smoke tests for the support headers in Geometry_Collision/: Constants.h,
// typeTraits_GM.h and Exceptions.h. The distance and intersection queries are
// tested in geometry_tests.cpp.

#include "Constants.h"
#include "Exceptions.h"
#include "typeTraits_GM.h"

#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace
{
    int g_failures = 0;

    void Check(bool condition, char const* what)
    {
        std::printf("[%s] %s\n", condition ? " OK " : "FAIL", what);
        if (!condition)
        {
            ++g_failures;
        }
    }

    bool Near(double a, double b, double tolerance = 1e-15)
    {
        return std::fabs(a - b) <= tolerance;
    }
}

int main()
{
    // typeTraits_GM.h
    Check(Vector_GM::has_division_operator<double>::value, "has_division_operator<double>");
    Check(Vector_GM::has_division_operator<float const>::value, "has_division_operator<float const>");
    Check(!Vector_GM::has_division_operator<int>::value, "!has_division_operator<int>");
    Check(!Vector_GM::is_arbitrary_precision<double>::value, "!is_arbitrary_precision<double>");

    // Constants.h: integer constants and n/d fractions.
    Check(Vector_GM::C_<float>(3) == 3.0f, "C_<float>(3) == 3");
    Check(Vector_GM::C_<double>(1, 4) == 0.25, "C_<double>(1, 4) == 0.25 (division type)");
    Check(Near(Vector_GM::C_<double>(1, 3), 1.0 / 3.0), "C_<double>(1, 3) == 1/3");
    // int has no division operator in the trait, so the n/d is computed in
    // double and truncated by the cast: 7/2 = 3.5 -> 3.
    Check(Vector_GM::C_<int>(7, 2) == 3, "C_<int>(7, 2) == 3 (non-division type)");

    // Constants.h: named constants.
    double const pi = std::acos(-1.0);
    Check(Near(Vector_GM::C_PI<double>, pi), "C_PI == acos(-1)");
    Check(Near(Vector_GM::C_TWO_PI<double>, 2.0 * pi), "C_TWO_PI == 2*pi");
    Check(Near(Vector_GM::C_PI_DIV_2<double>, 0.5 * pi), "C_PI_DIV_2 == pi/2");
    Check(Near(Vector_GM::C_DEG_TO_RAD<double> * 180.0, pi, 1e-13), "C_DEG_TO_RAD * 180 == pi");
    Check(Near(Vector_GM::C_DEG_TO_RAD<double> * Vector_GM::C_RAD_TO_DEG<double>, 1.0, 1e-14),
        "C_DEG_TO_RAD * C_RAD_TO_DEG == 1");
    Check(Near(Vector_GM::C_SQRT_2<double> * Vector_GM::C_INV_SQRT_2<double>, 1.0), "C_SQRT_2 * C_INV_SQRT_2 == 1");
    Check(Near(Vector_GM::C_LN_2<double> * Vector_GM::C_INV_LN_2<double>, 1.0), "C_LN_2 * C_INV_LN_2 == 1");

    // Exceptions.h: assertion macros throw the requested exception type and
    // put the message into what().
    bool threw = false;
    try
    {
        GTL_ARGUMENT_ASSERT(1 + 1 == 3, "bad argument");
    }
    catch (std::invalid_argument const& e)
    {
        threw = std::string(e.what()).find("bad argument") != std::string::npos;
    }
    Check(threw, "GTL_ARGUMENT_ASSERT(false) throws std::invalid_argument with message");

    threw = false;
    try
    {
        GTL_ARGUMENT_ASSERT(1 + 1 == 2, "never thrown");
    }
    catch (...)
    {
        threw = true;
    }
    Check(!threw, "GTL_ARGUMENT_ASSERT(true) does not throw");

    threw = false;
    try
    {
        GTL_RUNTIME_ERROR("runtime");
    }
    catch (std::runtime_error const&)
    {
        threw = true;
    }
    Check(threw, "GTL_RUNTIME_ERROR throws std::runtime_error");

    if (g_failures != 0)
    {
        std::printf("%d check(s) failed\n", g_failures);
        return 1;
    }
    std::printf("All checks passed\n");
    return 0;
}
