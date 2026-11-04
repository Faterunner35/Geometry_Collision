// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06
//
// Copied from GTE 6.x Mathematics/Hyperplane.h and modified for
// Geometry_Collision: namespace gte renamed to Vector_GM, includes point
// to the copies in this folder. Two bug fixes: operator< compared 'origin'
// where it should compare 'normal' (fixed upstream in GTE 6.0.2023.09.25),
// and the N != 3 constructor from points passed an invalid argument to
// SingularValueDecomposition::Solve, so it always threw.

#pragma once

#include "Vector.h"
#include <Mathematics/SingularValueDecomposition.h>
#include <array>
#include <cstdint>
#include <type_traits>

// The hyperplane is represented as Dot(U, X - P) = 0 where U is a unit-length
// normal vector, P is the hyperplane origin, and X is any point on the
// hyperplane. The user must ensure that the normal vector is unit length. The
// hyperplane constant is c = Dot(U, P) so that Dot(U, X) = c. If P is not
// specified when constructing a hyperplane, it is chosen to be the point on
// the plane closest to the origin, P = c * U.
//
// NOTE: You cannot set 'origin' and 'constant' independently. Use the
// constructors instead.
//
// // Construct from normal N and constant c.
// Plane3<T> plane(N, c);  // plane.origin = c * N
// 
// // Construct from normal N and origin P.
// Plane3<T> plane(N, P);  // plane.constant = Dot(N, P)
//
// Plane3<T> plane{};  // N = (0,0,0), P = (0,0,0), c = 0 [invalid]
// plane.normal = (0,0,1);
// plane.constant = 3;
// // If you consume plane now, the origin and constant are inconsistent
// // because P = (0,0,0) but Dot(N,P) = 0 != 3 = c. Instead use
// plane = Plane3<T>({ 0, 0, 1 }, 3);

namespace Vector_GM
{
	template <int32_t N, typename T>
	class Hyperplane
	{
	public:
		// Construction and destruction.  The default constructor sets the
		// normal to (0,...,0,1), the origin to (0,...,0) and the constant to
		// zero.
		Hyperplane()
			:
			normal{},
			origin(Vector<N, T>::Zero()),
			constant(static_cast<T>(0))
		{
			normal.MakeUnit(N - 1);
		}

		Hyperplane(Vector<N, T> const& inNormal, T const& inConstant)
			:
			normal(inNormal),
			origin(inConstant* inNormal),
			constant(inConstant)
		{
		}

		Hyperplane(Vector<N, T> const& inNormal, Vector<N, T> const& inOrigin)
			:
			normal(inNormal),
			origin(inOrigin),
			constant(Dot(inNormal, inOrigin))
		{
		}

		// U is a unit-length vector in the orthogonal complement of the set
		// {p[1]-p[0],...,p[n-1]-p[0]} and c = Dot(U,p[0]), where the p[i] are
		// pointson the hyperplane.
		Hyperplane(std::array<Vector<N, T>, N> const& p)
		{
			ComputeFromPoints<N>(p);
		}

		// Public member access.
		Vector<N, T> normal;
		Vector<N, T> origin;
		T constant;

	public:
		// Comparisons to support sorted containers.
		bool operator==(Hyperplane const& hyperplane) const
		{
			return normal == hyperplane.normal
				&& origin == hyperplane.origin
				&& constant == hyperplane.constant;
		}

		bool operator!=(Hyperplane const& hyperplane) const
		{
			return !operator==(hyperplane);
		}

		bool operator< (Hyperplane const& hyperplane) const
		{
			if (normal < hyperplane.normal)
			{
				return true;
			}

			if (normal > hyperplane.normal)
			{
				return false;
			}

			if (origin < hyperplane.origin)
			{
				return true;
			}

			if (origin > hyperplane.origin)
			{
				return false;
			}

			return constant < hyperplane.constant;
		}

		bool operator<=(Hyperplane const& hyperplane) const
		{
			return !hyperplane.operator<(*this);
		}

		bool operator> (Hyperplane const& hyperplane) const
		{
			return hyperplane.operator<(*this);
		}

		bool operator>=(Hyperplane const& hyperplane) const
		{
			return !operator<(hyperplane);
		}

	private:
		// For use in the Hyperplane(std::array<*>) constructor when N > 3.
		template <int32_t Dimension = N>
		typename std::enable_if<Dimension != 3, void>::type
			ComputeFromPoints(std::array<Vector<Dimension, T>, Dimension> const& p)
		{
			// Edges p[i]-p[0] as the columns of a Dimension x (Dimension-1)
			// matrix, stored row-major as SingularValueDecomposition expects.
			// (gte::Matrix storage depends on GTE_USE_ROW_MAJOR/COL_MAJOR, so it
			// is not used here.)
			std::array<T, Dimension * (Dimension - 1)> edge{};
			for (int32_t col = 0; col < Dimension - 1; ++col)
			{
				Vector<Dimension, T> e = p[col + 1] - p[0];
				for (int32_t row = 0; row < Dimension; ++row)
				{
					edge[row * (Dimension - 1) + col] = e[row];
				}
			}

			// Compute the 1-dimensional orthogonal complement of the edges of
			// the simplex formed by the points p[].
			gte::SingularValueDecomposition<T> svd(Dimension, Dimension - 1, 32);
			// GTE 6.x passed -1 as the second argument, which was the sort
			// flag of the older SVD interface. The current interface takes a
			// positive tolerance multiplier there (LogAssert fails on -1) and
			// always sorts the singular values in descending order, so the
			// last column of U spans the orthogonal complement.
			svd.Solve(edge.data());
			svd.GetUColumn(Dimension - 1, &normal[0]);

			constant = Dot(normal, p[0]);
			origin = constant * normal;
		}

		// For use in the Hyperplane(std::array<*>) constructor when N == 3.
		template <int32_t Dimension = N>
		typename std::enable_if<Dimension == 3, void>::type
			ComputeFromPoints(std::array<Vector<Dimension, T>, Dimension> const& p)
		{
			Vector<Dimension, T> edge0 = p[1] - p[0];
			Vector<Dimension, T> edge1 = p[2] - p[0];
			normal = UnitCross(edge0, edge1);
			constant = Dot(normal, p[0]);
			origin = constant * normal;
		}
	};

	// Template alias for convenience.
	template <typename T>
	using Plane3 = Hyperplane<3, T>;
}
