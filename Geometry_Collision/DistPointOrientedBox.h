#pragma once


#pragma once

#include "DistPointCanonicalBox.h"
#include "OrientedBox.h"

// Compute the distance from a point to a solid canonical box in nD.
// 
// The canonical box has center at the origin and is aligned with the
// coordinate axes. The extents are E = (e[0],e[1],...,e[n-1]). A box
// point is Y = (y[0],y[1],...,y[n-1]) with |y[i]| <= e[i] for all i.
// 
// The input point P is stored in closest[0]. The closest point on the box
// is stored in closest[1]. When there are infinitely many choices for the
// pair of closest points, only one of them is returned.

namespace Vector_GM
{
	template <int32_t N, typename T>
	class DCPQuery<T, Vector<N, T>, OrientedBox<N, T>>
	{
	public:
		using PCQuery = DCPQuery<T, Vector<N, T>, CanonicalBox<N, T>>;
		using Result = typename PCQuery::Result;

		Result operator()(Vector<N, T> const& point, OrientedBox<N, T> const& box)
		{
			Result result{};

			// Rotate and translate the point and box so that the box is
			// aligned and has center at the origin.
			CanonicalBox<N, T> cbox(box.extent);
			Vector<N, T> delta = point - box.center;
			Vector<N, T> xfrmPoint{};
			for (int32_t i = 0; i < N; ++i)
			{
				xfrmPoint[i] = Dot(box.axis[i], delta);
			}

			// The query computes 'result' relative to the box with center
			// at the origin.
			PCQuery pcQuery{};
			result = pcQuery(xfrmPoint, cbox);

			// Store the input point.
			result.closest[0] = point;

			// Rotate and translate the closest box point to the original
			// coordinates.
			Vector<N, T> closest1 = box.center;
			for (int32_t i = 0; i < N; ++i)
			{
				closest1 += result.closest[1][i] * box.axis[i];
			}
			result.closest[1] = closest1;

			return result;
		}
	};

	// Template aliases for convenience.
	template <int32_t N, typename T>
	using DCPPointOrientedBox = DCPQuery<T, Vector<N, T>, OrientedBox<N, T>>;

	template <typename T>
	using DCPPoint2OrientedBox2 = DCPPointOrientedBox<2, T>;

	template <typename T>
	using DCPPoint3OrientedBox3 = DCPPointOrientedBox<3, T>;
}