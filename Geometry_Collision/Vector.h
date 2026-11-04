// Vector types for namespace Vector_GM.
//
// Geometry_Collision uses exactly one vector type: GTE's Vector<N, Real>
// (dimension first, then the scalar type), defined in the vendored
// Mathematics/Vector.h by David Eberly, Geometric Tools (Boost Software
// License 1.0, see licenses/BSL-1.0.txt). This header does not define a
// second vector class. It makes GTE's vector class templates and their free
// functions visible in namespace Vector_GM, so that the geometry headers in
// this folder (all in namespace Vector_GM) can write Vector<N, T>,
// Vector3<T>, Dot, Cross, ... and still interoperate with the GTE matrix,
// quaternion and graphics code, which use the same type.
//
// Unqualified calls such as Dot(u, v) also work without these
// using-declarations (argument-dependent lookup finds gte::Dot); the
// declarations are there so that qualified names like Vector_GM::Dot and
// Vector_GM::Vector3<float> are valid.
//
// Mathematics/ is not on the include path. Its headers are included as
// <Mathematics/...> with the parent folder (Geometry_Collision/) on the
// include path, which is the layout GTE expects. This keeps GTE's
// Mathematics/Math.h from shadowing the system <math.h>.

#pragma once

#include <Mathematics/Vector.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/Vector4.h>

namespace Vector_GM
{
    // Class templates and aliases.
    using gte::Vector;
    using gte::Vector2;
    using gte::Vector3;
    using gte::Vector4;

    // Free functions for Vector<N, Real> (Mathematics/Vector.h).
    using gte::Dot;
    using gte::Length;
    using gte::Normalize;
    using gte::Orthonormalize;
    using gte::GetOrthogonal;
    using gte::ComputeExtremes;
    using gte::HLift;
    using gte::HProject;
    using gte::Lift;
    using gte::Project;

    // Free functions for Vector2/Vector3/Vector4.
    using gte::Perp;
    using gte::UnitPerp;
    using gte::DotPerp;
    using gte::Cross;
    using gte::UnitCross;
    using gte::DotCross;
    using gte::HyperCross;
    using gte::UnitHyperCross;
    using gte::DotHyperCross;
    using gte::ComputeOrthogonalComplement;
    using gte::ComputeBarycentrics;
}
