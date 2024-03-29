
#pragma once

#include "Vector.h"

namespace Vector_GM
{
    // Axis-angle representation for N = 3 or N = 4.  When N = 4, the axis
    // must be a vector of the form (x,y,z,0) [affine representation of the
    // 3-tuple direction].

    template <int32_t N, typename Real>
    class AxisAngle
    {
    public:
        AxisAngle()
            :
            axis(Vector<N, Real>::Zero()),
            angle((Real)0)
        {
            static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
        }

        AxisAngle(Vector<N, Real> const& inAxis, Real inAngle)
            :
            axis(inAxis),
            angle(inAngle)
        {
            static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
        }

        Vector<N, Real> axis;
        Real angle;
    };
}
