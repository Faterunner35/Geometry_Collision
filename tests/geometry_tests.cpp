// Numeric tests for the distance and intersection queries in
// Geometry_Collision/ (namespace Vector_GM). Every expected value is computed
// by hand in the comment next to it; the inputs are chosen so that most
// expected values are exact in binary floating point. Edge cases covered:
// inside, touching (distance equal to the radius), just separated, corner,
// edge and face features, degenerate boxes.

#include "test_harness.h"

#include "AlignedBox.h"
#include "CanonicalBox.h"
#include "DistPointAlignedBox.h"
#include "DistPointCanonicalBox.h"
#include "DistPointOrientedBox.h"
#include "HyperPlane.h"
#include "HyperSphere.h"
#include "IntrAlignedBoxSphere.h"
#include "IntrIntervals.h"
#include "IntrLineAlignedBox.h"
#include "IntrOrientedBoxShere.h"
#include "IntrRayAlignedBox.h"
#include "Line.h"
#include "OrientedBox.h"
#include "Ray.h"

#include <array>
#include <cmath>

using namespace Vector_GM;

namespace
{
    double const eps = 1e-12;
    double const s = 1.0 / std::sqrt(2.0);      // cos(45 deg) = sin(45 deg)
    double const sqrt2 = std::sqrt(2.0);
    double const sqrt3 = std::sqrt(3.0);

    // The cube [-1,1]^3 used by most box tests.
    AlignedBox3<double> UnitCube()
    {
        return AlignedBox3<double>({ -1.0, -1.0, -1.0 }, { 1.0, 1.0, 1.0 });
    }

    // The cube [-1,1]^3 rotated by 45 degrees about the z-axis:
    // U0 = (s,s,0), U1 = (-s,s,0), U2 = (0,0,1), extents (1,1,1). Its
    // vertical edge with box coordinates (y0,y1) = (1,-1) is the line
    // x = sqrt(2), y = 0, the point of the box farthest along +x.
    OrientedBox3<double> RotatedCube(Vector3<double> const& center)
    {
        std::array<Vector3<double>, 3> axis{};
        axis[0] = { s, s, 0.0 };
        axis[1] = { -s, s, 0.0 };
        axis[2] = { 0.0, 0.0, 1.0 };
        return OrientedBox3<double>(center, axis, { 1.0, 1.0, 1.0 });
    }

    void TestDistPointCanonicalBox()
    {
        gc_test::Section("DCPQuery point / canonical box");
        DCPQuery<double, Vector3<double>, CanonicalBox3<double>> query;
        CanonicalBox3<double> box({ 1.0, 2.0, 3.0 });   // |x|<=1, |y|<=2, |z|<=3

        // Center: inside, distance 0, closest point is the point itself.
        auto r = query({ 0.0, 0.0, 0.0 }, box);
        GC_CHECK_NEAR(r.distance, 0.0, 0.0);
        GC_CHECK_VEC3(r.closest[1], 0.0, 0.0, 0.0, 0.0);

        // (3,0,0): only x is outside, clamp to x = 1. Distance 3 - 1 = 2.
        r = query({ 3.0, 0.0, 0.0 }, box);
        GC_CHECK_NEAR(r.sqrDistance, 4.0, eps);
        GC_CHECK_NEAR(r.distance, 2.0, eps);
        GC_CHECK_VEC3(r.closest[0], 3.0, 0.0, 0.0, 0.0);
        GC_CHECK_VEC3(r.closest[1], 1.0, 0.0, 0.0, eps);

        // (2,4,6): outside in every coordinate, closest is the corner
        // (1,2,3). Difference (1,2,3), squared distance 1 + 4 + 9 = 14.
        r = query({ 2.0, 4.0, 6.0 }, box);
        GC_CHECK_NEAR(r.sqrDistance, 14.0, eps);
        GC_CHECK_NEAR(r.distance, std::sqrt(14.0), eps);
        GC_CHECK_VEC3(r.closest[1], 1.0, 2.0, 3.0, eps);

        // The corner itself is on the box: distance 0.
        r = query({ 1.0, 2.0, 3.0 }, box);
        GC_CHECK_NEAR(r.distance, 0.0, 0.0);

        // (-4,-2,0): x clamps to -1 (difference 3); y = -2 is on the face.
        r = query({ -4.0, -2.0, 0.0 }, box);
        GC_CHECK_NEAR(r.distance, 3.0, eps);
        GC_CHECK_VEC3(r.closest[1], -1.0, -2.0, 0.0, eps);

        // Degenerate box (all extents 0) is the origin: distance to (3,4,0)
        // is 5.
        CanonicalBox3<double> point({ 0.0, 0.0, 0.0 });
        r = query({ 3.0, 4.0, 0.0 }, point);
        GC_CHECK_NEAR(r.distance, 5.0, eps);

        // 2D: square |x|,|y| <= 1 and P = (2,-3): closest (1,-1),
        // squared distance 1 + 4 = 5.
        DCPQuery<double, Vector2<double>, CanonicalBox2<double>> query2;
        auto r2 = query2({ 2.0, -3.0 }, CanonicalBox2<double>({ 1.0, 1.0 }));
        GC_CHECK_NEAR(r2.sqrDistance, 5.0, eps);
        GC_CHECK_NEAR(r2.closest[1][0], 1.0, eps);
        GC_CHECK_NEAR(r2.closest[1][1], -1.0, eps);
    }

    void TestDistPointAlignedBox()
    {
        gc_test::Section("DCPQuery point / aligned box");
        DCPQuery<double, Vector3<double>, AlignedBox3<double>> query;
        // min (1,1,1), max (3,5,7): center (2,3,4), extents (1,2,3).
        AlignedBox3<double> box({ 1.0, 1.0, 1.0 }, { 3.0, 5.0, 7.0 });

        // Origin: below every face, closest is the min corner, distance
        // sqrt(1 + 1 + 1) = sqrt(3).
        auto r = query({ 0.0, 0.0, 0.0 }, box);
        GC_CHECK_NEAR(r.distance, sqrt3, eps);
        GC_CHECK_VEC3(r.closest[0], 0.0, 0.0, 0.0, 0.0);
        GC_CHECK_VEC3(r.closest[1], 1.0, 1.0, 1.0, eps);

        // Box center: inside.
        r = query({ 2.0, 3.0, 4.0 }, box);
        GC_CHECK_NEAR(r.distance, 0.0, eps);
        GC_CHECK_VEC3(r.closest[1], 2.0, 3.0, 4.0, eps);

        // (5,3,4): in front of the x = 3 face, distance 2.
        r = query({ 5.0, 3.0, 4.0 }, box);
        GC_CHECK_NEAR(r.distance, 2.0, eps);
        GC_CHECK_VEC3(r.closest[1], 3.0, 3.0, 4.0, eps);

        // Max corner: on the box.
        r = query({ 3.0, 5.0, 7.0 }, box);
        GC_CHECK_NEAR(r.distance, 0.0, eps);

        // (2,10,4): beyond the y = 5 face, distance 5.
        r = query({ 2.0, 10.0, 4.0 }, box);
        GC_CHECK_NEAR(r.distance, 5.0, eps);
        GC_CHECK_VEC3(r.closest[1], 2.0, 5.0, 4.0, eps);
    }

    void TestDistPointOrientedBox()
    {
        gc_test::Section("DCPQuery point / oriented box");
        DCPQuery<double, Vector3<double>, OrientedBox3<double>> query;
        Vector3<double> C{ 1.0, 1.0, 0.0 };
        auto box = RotatedCube(C);
        auto const& U0 = box.axis[0];
        auto const& U1 = box.axis[1];

        // P = C + 3*U0 has box coordinates (3,0,0); closest box point has
        // coordinates (1,0,0), i.e. C + U0 = (1+s, 1+s, 0). Distance 2.
        auto r = query(C + 3.0 * U0, box);
        GC_CHECK_NEAR(r.distance, 2.0, eps);
        GC_CHECK_VEC3(r.closest[1], 1.0 + s, 1.0 + s, 0.0, eps);

        // P = C + 2*U0 + 2*U1 = (1, 1 + 2*sqrt(2), 0) has box coordinates
        // (2,2,0); closest (1,1,0) -> C + U0 + U1 = (1, 1 + sqrt(2), 0).
        // Distance |(1,1)| = sqrt(2).
        r = query(C + 2.0 * U0 + 2.0 * U1, box);
        GC_CHECK_NEAR(r.distance, sqrt2, eps);
        GC_CHECK_VEC3(r.closest[1], 1.0, 1.0 + sqrt2, 0.0, eps);

        // Inside: P = C + 0.5*U1.
        r = query(C + 0.5 * U1, box);
        GC_CHECK_NEAR(r.distance, 0.0, eps);

        // Above the top face: P = C + (0,0,5), closest (1,1,1), distance 4.
        r = query(C + Vector3<double>{ 0.0, 0.0, 5.0 }, box);
        GC_CHECK_NEAR(r.distance, 4.0, eps);
        GC_CHECK_VEC3(r.closest[1], 1.0, 1.0, 1.0, eps);

        // An oriented box with the standard axes is the aligned box
        // min (1,1,1), max (3,5,7): same answer as the aligned query.
        OrientedBox3<double> obb;   // axes = standard basis
        obb.center = { 2.0, 3.0, 4.0 };
        obb.extent = { 1.0, 2.0, 3.0 };
        r = query({ 0.0, 0.0, 0.0 }, obb);
        GC_CHECK_NEAR(r.distance, sqrt3, eps);
        GC_CHECK_VEC3(r.closest[1], 1.0, 1.0, 1.0, eps);
    }

    void TestTIAlignedBoxSphere()
    {
        gc_test::Section("TIQuery aligned box / sphere");
        TIQuery<double, AlignedBox3<double>, Sphere3<double>> query;
        auto box = UnitCube();

        // Face: center (3,0,0) is at distance 2 from the face x = 1.
        // r = 2 touches (the query uses <=), r slightly smaller separates.
        GC_CHECK(query(box, Sphere3<double>({ 3.0, 0.0, 0.0 }, 2.0)).intersect);
        GC_CHECK(!query(box, Sphere3<double>({ 3.0, 0.0, 0.0 }, 1.999)).intersect);

        // Sphere strictly inside the box, and box strictly inside the
        // sphere: the objects are solids, so both intersect.
        GC_CHECK(query(box, Sphere3<double>({ 0.0, 0.0, 0.0 }, 0.5)).intersect);
        GC_CHECK(query(box, Sphere3<double>({ 0.0, 0.0, 0.0 }, 10.0)).intersect);

        // Corner: center (2,2,2) is sqrt(3) = 1.7320508 from (1,1,1).
        GC_CHECK(query(box, Sphere3<double>({ 2.0, 2.0, 2.0 }, 1.75)).intersect);
        GC_CHECK(!query(box, Sphere3<double>({ 2.0, 2.0, 2.0 }, 1.7)).intersect);

        // Edge: center (2,2,0) is sqrt(2) = 1.4142136 from the edge
        // x = y = 1.
        GC_CHECK(query(box, Sphere3<double>({ 2.0, 2.0, 0.0 }, 1.42)).intersect);
        GC_CHECK(!query(box, Sphere3<double>({ 2.0, 2.0, 0.0 }, 1.41)).intersect);
    }

    void TestTIOrientedBoxSphere()
    {
        gc_test::Section("TIQuery oriented box / sphere");
        TIQuery<double, OrientedBox3<double>, Sphere3<double>> query;
        TIQuery<double, AlignedBox3<double>, Sphere3<double>> aabbQuery;
        auto box = RotatedCube({ 0.0, 0.0, 0.0 });

        // Center (sqrt(2) + 0.5, 0, 0) is 0.5 from the box edge at
        // x = sqrt(2). sqrt(2) is not exact in binary, so "touching" is
        // tested with a radius 1e-9 above and below 0.5.
        Vector3<double> c{ sqrt2 + 0.5, 0.0, 0.0 };
        GC_CHECK(query(box, Sphere3<double>(c, 0.5 + 1e-9)).intersect);
        GC_CHECK(!query(box, Sphere3<double>(c, 0.5 - 1e-9)).intersect);
        // The unrotated cube [-1,1]^3 is sqrt(2) - 0.5 = 0.914 away.
        GC_CHECK(!aabbQuery(UnitCube(), Sphere3<double>(c, 0.5)).intersect);

        // (1.2,0,0) has box coordinates (0.85,-0.85,0): inside the rotated
        // cube, but 0.2 outside the unrotated one.
        Sphere3<double> small({ 1.2, 0.0, 0.0 }, 0.01);
        GC_CHECK(query(box, small).intersect);
        GC_CHECK(!aabbQuery(UnitCube(), small).intersect);

        // Top face z = 1: center (0,0,2.5) is exactly 1.5 above it.
        GC_CHECK(query(box, Sphere3<double>({ 0.0, 0.0, 2.5 }, 1.5)).intersect);
        GC_CHECK(!query(box, Sphere3<double>({ 0.0, 0.0, 2.5 }, 1.49)).intersect);
    }

    void TestFIMovingAlignedBoxSphere()
    {
        gc_test::Section("FIQuery moving sphere / aligned box");
        FIQuery<double, AlignedBox3<double>, Sphere3<double>> query;
        auto box = UnitCube();
        Vector3<double> const zero{ 0.0, 0.0, 0.0 };

        // Face contact: sphere radius 1 at (5,0,0) moving with (-1,0,0).
        // It touches x = 1 when its center is at x = 2: t = 3, contact
        // point (1,0,0).
        auto r = query(box, zero, Sphere3<double>({ 5.0, 0.0, 0.0 }, 1.0), { -1.0, 0.0, 0.0 });
        GC_CHECK(r.intersectionType == 1);
        GC_CHECK_NEAR(r.contactTime, 3.0, eps);
        GC_CHECK_VEC3(r.contactPoint, 1.0, 0.0, 0.0, eps);

        // Moving away: no contact.
        r = query(box, zero, Sphere3<double>({ 5.0, 0.0, 0.0 }, 1.0), { 1.0, 0.0, 0.0 });
        GC_CHECK(r.intersectionType == 0);

        // Not moving and separated: no contact.
        r = query(box, zero, Sphere3<double>({ 5.0, 0.0, 0.0 }, 1.0), zero);
        GC_CHECK(r.intersectionType == 0);

        // Initially overlapping, center inside the box: type -1, time 0,
        // contact point = sphere center.
        r = query(box, zero, Sphere3<double>({ 0.5, 0.0, 0.0 }, 1.0), { -1.0, 0.0, 0.0 });
        GC_CHECK(r.intersectionType == -1);
        GC_CHECK_NEAR(r.contactTime, 0.0, 0.0);
        GC_CHECK_VEC3(r.contactPoint, 0.5, 0.0, 0.0, eps);

        // Initially overlapping, center (1.5,0,0) outside the face x = 1:
        // type -1, time 0. The header comment says the contact point is the
        // sphere center, but the code (FaceOverlap) returns the closest box
        // point, (1,0,0).
        r = query(box, zero, Sphere3<double>({ 1.5, 0.0, 0.0 }, 1.0), { -1.0, 0.0, 0.0 });
        GC_CHECK(r.intersectionType == -1);
        GC_CHECK_NEAR(r.contactTime, 0.0, 0.0);
        GC_CHECK_VEC3(r.contactPoint, 1.0, 0.0, 0.0, eps);

        // Touching at time 0 (center (2,0,0), radius 1, at rest): not an
        // overlap (-1) but a contact at t = 0 at (1,0,0).
        r = query(box, zero, Sphere3<double>({ 2.0, 0.0, 0.0 }, 1.0), zero);
        GC_CHECK(r.intersectionType == 1);
        GC_CHECK_NEAR(r.contactTime, 0.0, 0.0);
        GC_CHECK_VEC3(r.contactPoint, 1.0, 0.0, 0.0, eps);

        // Vertex contact: center (3,3,3) moving with (-1,-1,-1). The center
        // is (1+u,1+u,1+u) at time 2-u and touches the vertex (1,1,1) when
        // u*sqrt(3) = 1, so t = 2 - 1/sqrt(3) = 1.42264973.
        r = query(box, zero, Sphere3<double>({ 3.0, 3.0, 3.0 }, 1.0), { -1.0, -1.0, -1.0 });
        GC_CHECK(r.intersectionType == 1);
        GC_CHECK_NEAR(r.contactTime, 2.0 - 1.0 / sqrt3, 1e-9);
        GC_CHECK_VEC3(r.contactPoint, 1.0, 1.0, 1.0, 1e-9);

        // Edge contact: center (3,3,0) moving with (-1,-1,0) hits the edge
        // x = y = 1 when u*sqrt(2) = 1: t = 2 - 1/sqrt(2) = 1.29289322.
        r = query(box, zero, Sphere3<double>({ 3.0, 3.0, 0.0 }, 1.0), { -1.0, -1.0, 0.0 });
        GC_CHECK(r.intersectionType == 1);
        GC_CHECK_NEAR(r.contactTime, 2.0 - s, 1e-9);
        GC_CHECK_VEC3(r.contactPoint, 1.0, 1.0, 0.0, 1e-9);

        // Miss: the path y = 3 passes 2 above the face y = 1 (radius 1).
        r = query(box, zero, Sphere3<double>({ 5.0, 3.0, 0.0 }, 1.0), { -1.0, 0.0, 0.0 });
        GC_CHECK(r.intersectionType == 0);

        // Moving box, static sphere: only the relative velocity matters,
        // so t = 3 as in the face case. The contact point is reported in
        // the box's frame at time 0 (box center + relative contact), which
        // is (1,0,0); in world coordinates at the contact time it is
        // (1,0,0) + t * boxVelocity = (4,0,0), the point of the sphere
        // nearest the box.
        Vector3<double> boxVelocity{ 1.0, 0.0, 0.0 };
        r = query(box, boxVelocity, Sphere3<double>({ 5.0, 0.0, 0.0 }, 1.0), zero);
        GC_CHECK(r.intersectionType == 1);
        GC_CHECK_NEAR(r.contactTime, 3.0, eps);
        GC_CHECK_VEC3(r.contactPoint, 1.0, 0.0, 0.0, eps);
        auto worldContact = r.contactPoint + r.contactTime * boxVelocity;
        GC_CHECK_VEC3(worldContact, 4.0, 0.0, 0.0, eps);
    }

    void TestFIMovingOrientedBoxSphere()
    {
        gc_test::Section("FIQuery moving sphere / oriented box");
        FIQuery<double, OrientedBox3<double>, Sphere3<double>> query;
        auto box = RotatedCube({ 0.0, 0.0, 0.0 });
        Vector3<double> const zero{ 0.0, 0.0, 0.0 };

        // Sphere radius 0.5 at (5,0,0) moving with (-1,0,0) first touches
        // the vertical edge at x = sqrt(2) when its center is at
        // x = sqrt(2) + 0.5: t = 4.5 - sqrt(2) = 3.08578644.
        auto r = query(box, zero, Sphere3<double>({ 5.0, 0.0, 0.0 }, 0.5), { -1.0, 0.0, 0.0 });
        GC_CHECK(r.intersectionType == 1);
        GC_CHECK_NEAR(r.contactTime, 4.5 - sqrt2, 1e-9);
        GC_CHECK_VEC3(r.contactPoint, sqrt2, 0.0, 0.0, 1e-9);

        // Moving away.
        r = query(box, zero, Sphere3<double>({ 5.0, 0.0, 0.0 }, 0.5), { 1.0, 0.0, 0.0 });
        GC_CHECK(r.intersectionType == 0);

        // Initially overlapping (center inside the box).
        r = query(box, zero, Sphere3<double>({ 0.5, 0.0, 0.0 }, 0.25), { 1.0, 0.0, 0.0 });
        GC_CHECK(r.intersectionType == -1);
    }

    void TestLineAlignedBox()
    {
        gc_test::Section("TIQuery/FIQuery line / aligned box");
        TIQuery<double, Line3<double>, AlignedBox3<double>> tiQuery;
        FIQuery<double, Line3<double>, AlignedBox3<double>> fiQuery;
        auto box = UnitCube();

        // Along the x-axis from (-5,0,0): enters x = -1 at t = 4, leaves
        // x = 1 at t = 6.
        Line3<double> line({ -5.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 });
        GC_CHECK(tiQuery(line, box).intersect);
        auto r = fiQuery(line, box);
        GC_CHECK(r.intersect);
        GC_CHECK(r.numIntersections == 2);
        GC_CHECK_NEAR(r.parameter[0], 4.0, eps);
        GC_CHECK_NEAR(r.parameter[1], 6.0, eps);
        GC_CHECK_VEC3(r.point[0], -1.0, 0.0, 0.0, eps);
        GC_CHECK_VEC3(r.point[1], 1.0, 0.0, 0.0, eps);

        // Parallel to x at height y = 2: misses.
        line = Line3<double>({ 0.0, 2.0, 0.0 }, { 1.0, 0.0, 0.0 });
        GC_CHECK(!tiQuery(line, box).intersect);
        r = fiQuery(line, box);
        GC_CHECK(!r.intersect);
        GC_CHECK(r.numIntersections == 0);

        // Lying in the face y = 1: the whole segment x in [-1,1] is on the
        // (solid) box, t in [4,6].
        line = Line3<double>({ -5.0, 1.0, 0.0 }, { 1.0, 0.0, 0.0 });
        GC_CHECK(tiQuery(line, box).intersect);
        r = fiQuery(line, box);
        GC_CHECK(r.numIntersections == 2);
        GC_CHECK_VEC3(r.point[0], -1.0, 1.0, 0.0, eps);
        GC_CHECK_VEC3(r.point[1], 1.0, 1.0, 0.0, eps);

        // Through the corner (1,1,1) with direction (s,-s,0): the points
        // (1+t, 1-t, 1) are in the box only for t = 0, a single point.
        line = Line3<double>({ 1.0, 1.0, 1.0 }, { s, -s, 0.0 });
        GC_CHECK(tiQuery(line, box).intersect);
        r = fiQuery(line, box);
        GC_CHECK(r.intersect);
        GC_CHECK(r.numIntersections == 1);
        GC_CHECK_NEAR(r.parameter[0], 0.0, eps);
        GC_CHECK_VEC3(r.point[0], 1.0, 1.0, 1.0, eps);

        // Box not centered at the origin: min (2,2,2), max (4,6,8). The
        // line (0,4,5) + t(1,0,0) is in the box for t in [2,4].
        AlignedBox3<double> offsetBox({ 2.0, 2.0, 2.0 }, { 4.0, 6.0, 8.0 });
        line = Line3<double>({ 0.0, 4.0, 5.0 }, { 1.0, 0.0, 0.0 });
        r = fiQuery(line, offsetBox);
        GC_CHECK(r.numIntersections == 2);
        GC_CHECK_NEAR(r.parameter[0], 2.0, eps);
        GC_CHECK_NEAR(r.parameter[1], 4.0, eps);
        GC_CHECK_VEC3(r.point[0], 2.0, 4.0, 5.0, eps);
        GC_CHECK_VEC3(r.point[1], 4.0, 4.0, 5.0, eps);
    }

    void TestRayAlignedBox()
    {
        gc_test::Section("TIQuery/FIQuery ray / aligned box");
        TIQuery<double, Ray3<double>, AlignedBox3<double>> tiQuery;
        FIQuery<double, Ray3<double>, AlignedBox3<double>> fiQuery;
        auto box = UnitCube();

        // Toward the box from (-5,0,0): t in [4,6] as for the line.
        Ray3<double> ray({ -5.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 });
        GC_CHECK(tiQuery(ray, box).intersect);
        auto r = fiQuery(ray, box);
        GC_CHECK(r.numIntersections == 2);
        GC_CHECK_NEAR(r.parameter[0], 4.0, eps);
        GC_CHECK_NEAR(r.parameter[1], 6.0, eps);

        // Same origin, pointing away: the line hits the box at t in [-6,-4],
        // which is not on the ray (t >= 0).
        ray = Ray3<double>({ -5.0, 0.0, 0.0 }, { -1.0, 0.0, 0.0 });
        GC_CHECK(!tiQuery(ray, box).intersect);
        r = fiQuery(ray, box);
        GC_CHECK(!r.intersect);
        GC_CHECK(r.numIntersections == 0);

        // Starting inside at the center: line interval [-1,1] clipped to
        // [0,1]; the points are the origin and the exit point (1,0,0).
        ray = Ray3<double>({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 });
        GC_CHECK(tiQuery(ray, box).intersect);
        r = fiQuery(ray, box);
        GC_CHECK(r.numIntersections == 2);
        GC_CHECK_NEAR(r.parameter[0], 0.0, eps);
        GC_CHECK_NEAR(r.parameter[1], 1.0, eps);
        GC_CHECK_VEC3(r.point[0], 0.0, 0.0, 0.0, eps);
        GC_CHECK_VEC3(r.point[1], 1.0, 0.0, 0.0, eps);

        // Starting on the face x = 1 and leaving: line interval [-2,0], ray
        // interval [0,inf); they touch at t = 0, one intersection point.
        ray = Ray3<double>({ 1.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 });
        GC_CHECK(tiQuery(ray, box).intersect);
        r = fiQuery(ray, box);
        GC_CHECK(r.intersect);
        GC_CHECK(r.numIntersections == 1);
        GC_CHECK_NEAR(r.parameter[0], 0.0, eps);
        GC_CHECK_VEC3(r.point[0], 1.0, 0.0, 0.0, eps);

        // Starting outside at (2,0,0) and leaving: no intersection.
        ray = Ray3<double>({ 2.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 });
        GC_CHECK(!tiQuery(ray, box).intersect);
        GC_CHECK(!fiQuery(ray, box).intersect);
    }

    void TestIntervals()
    {
        gc_test::Section("FIQuery interval / interval");
        FIQuery<double, std::array<double, 2>, std::array<double, 2>> query;

        // [0,2] and [1,3] overlap in [1,2].
        auto r = query({ 0.0, 2.0 }, { 1.0, 3.0 });
        GC_CHECK(r.intersect);
        GC_CHECK(r.numIntersections == 2);
        GC_CHECK_NEAR(r.overlap[0], 1.0, 0.0);
        GC_CHECK_NEAR(r.overlap[1], 2.0, 0.0);

        // [0,1] and [1,2] touch at 1.
        r = query({ 0.0, 1.0 }, { 1.0, 2.0 });
        GC_CHECK(r.intersect);
        GC_CHECK(r.numIntersections == 1);
        GC_CHECK_NEAR(r.overlap[0], 1.0, 0.0);

        // [0,1] and [2,3] are disjoint.
        r = query({ 0.0, 1.0 }, { 2.0, 3.0 });
        GC_CHECK(!r.intersect);
        GC_CHECK(r.numIntersections == 0);
    }

    void TestHyperplane()
    {
        gc_test::Section("Hyperplane construction");

        // Normal (0,0,1) and constant 2: origin = 2 * normal.
        Plane3<double> plane({ 0.0, 0.0, 1.0 }, 2.0);
        GC_CHECK_VEC3(plane.origin, 0.0, 0.0, 2.0, 0.0);
        GC_CHECK_NEAR(plane.constant, 2.0, 0.0);

        // Normal and a point on the plane: constant = Dot(N, P) = 2.
        plane = Plane3<double>({ 0.0, 0.0, 1.0 }, Vector3<double>{ 5.0, 5.0, 2.0 });
        GC_CHECK_NEAR(plane.constant, 2.0, 0.0);

        // Three points on z = 1: edges (1,0,0), (0,1,0), normal is their
        // unit cross product (0,0,1), constant 1.
        plane = Plane3<double>(std::array<Vector3<double>, 3>{
            Vector3<double>{ 0.0, 0.0, 1.0 },
            Vector3<double>{ 1.0, 0.0, 1.0 },
            Vector3<double>{ 0.0, 1.0, 1.0 } });
        GC_CHECK_VEC3(plane.normal, 0.0, 0.0, 1.0, eps);
        GC_CHECK_NEAR(plane.constant, 1.0, eps);

        // 4D (the SVD code path): four points on x3 = 2. The normal is
        // +-(0,0,0,1) and constant = Dot(normal, p0) has the same sign, so
        // origin = constant * normal = (0,0,0,2) either way.
        Hyperplane<4, double> h4(std::array<Vector4<double>, 4>{
            Vector4<double>{ 0.0, 0.0, 0.0, 2.0 },
            Vector4<double>{ 1.0, 0.0, 0.0, 2.0 },
            Vector4<double>{ 0.0, 1.0, 0.0, 2.0 },
            Vector4<double>{ 0.0, 0.0, 1.0, 2.0 } });
        GC_CHECK_NEAR(std::fabs(h4.normal[3]), 1.0, 1e-9);
        GC_CHECK_NEAR(h4.normal[0], 0.0, 1e-9);
        GC_CHECK_NEAR(h4.normal[1], 0.0, 1e-9);
        GC_CHECK_NEAR(h4.normal[2], 0.0, 1e-9);
        GC_CHECK_NEAR(h4.origin[3], 2.0, 1e-9);
        GC_CHECK_NEAR(h4.constant * h4.normal[3], 2.0, 1e-9);
    }

    void TestFloat()
    {
        // The demos use float. One face-contact case in float.
        gc_test::Section("float instantiation");
        FIQuery<float, AlignedBox3<float>, Sphere3<float>> query;
        AlignedBox3<float> box({ -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f });
        auto r = query(box, { 0.0f, 0.0f, 0.0f },
            Sphere3<float>({ 5.0f, 0.0f, 0.0f }, 1.0f), { -1.0f, 0.0f, 0.0f });
        GC_CHECK(r.intersectionType == 1);
        GC_CHECK_NEAR(r.contactTime, 3.0f, 1e-5f);
    }
}

int main()
{
    TestDistPointCanonicalBox();
    TestDistPointAlignedBox();
    TestDistPointOrientedBox();
    TestTIAlignedBoxSphere();
    TestTIOrientedBoxSphere();
    TestFIMovingAlignedBoxSphere();
    TestFIMovingOrientedBoxSphere();
    TestLineAlignedBox();
    TestRayAlignedBox();
    TestIntervals();
    TestHyperplane();
    TestFloat();
    return gc_test::Summary();
}
