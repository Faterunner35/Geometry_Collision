#pragma once



#include "Vector.h"

// The ray is represented as P+t*D, where P is the ray origin, D is a
// unit-length direction vector, and t >= 0.  The user must ensure that D is
// unit length.

namespace Vector_GM {
    template <int32_t N, typename Real>
    class Ray
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // origin to (0,...,0) and the ray direction to (1,0,...,0).
        Ray()
        {
            origin.MakeZero();
            direction.MakeUnit(0);
        }

        Ray(Vector<N, Real> const& inOrigin, Vector<N, Real> const& inDirection)
            :
            origin(inOrigin),
            direction(inDirection)
        {
        }

        // Public member access.  The direction must be unit length.
        Vector<N, Real> origin, direction;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Ray const& ray) const
        {
            return origin == ray.origin && direction == ray.direction;
        }

        bool operator!=(Ray const& ray) const
        {
            return !operator==(ray);
        }

        bool operator< (Ray const& ray) const
        {
            if (origin < ray.origin)
            {
                return true;
            }

            if (origin > ray.origin)
            {
                return false;
            }

            return direction < ray.direction;
        }

        bool operator<=(Ray const& ray) const
        {
            return !ray.operator<(*this);
        }

        bool operator> (Ray const& ray) const
        {
            return ray.operator<(*this);
        }

        bool operator>=(Ray const& ray) const
        {
            return !operator<(ray);
        }
    };

    // Template aliases for convenience.
    template <typename Real>
    using Ray2 = Ray<2, Real>;

    template <typename Real>
    using Ray3 = Ray<3, Real>;
}

