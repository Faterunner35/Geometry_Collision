#pragma once
// The hypersphere is represented as |X-C| = R where C is the center and R is
// the radius. The hypersphere is a circle for dimension 2 or a sphere for
// dimension 3.

#include "Vector.h"
#include <cstddef>

namespace Vector_GM
{
	template <typename T, size_t N>
	class Hypersphere
	{
	public:
		using value_type = T;

		// Construction. The default constructor sets all members to zero.
		Hypersphere()
			:
			center{},
			radius(C_<T>(0))
		{
		}

		Hypersphere(Vector<T, N> const& inCenter, T const& inRadius)
			:
			center(inCenter),
			radius(inRadius)
		{
		}

		Vector<T, N> center;
		T radius;

	private:
		friend class UnitTestHypersphere;
	};

	// Comparisons to support sorted containers.
	template <typename T, size_t N>
	bool operator==(Hypersphere<T, N> const& hypersphere0, Hypersphere<T, N> const& hypersphere1)
	{
		return hypersphere0.center == hypersphere1.center
			&& hypersphere0.radius == hypersphere1.radius;
	}

	template <typename T, size_t N>
	bool operator!=(Hypersphere<T, N> const& hypersphere0, Hypersphere<T, N> const& hypersphere1)
	{
		return !operator==(hypersphere0, hypersphere1);
	}

	template <typename T, size_t N>
	bool operator<(Hypersphere<T, N> const& hypersphere0, Hypersphere<T, N> const& hypersphere1)
	{
		if (hypersphere0.center < hypersphere1.center)
		{
			return true;
		}

		if (hypersphere0.center > hypersphere1.center)
		{
			return false;
		}

		return hypersphere0.radius < hypersphere1.radius;
	}

	template <typename T, size_t N>
	bool operator<=(Hypersphere<T, N> const& hypersphere0, Hypersphere<T, N> const& hypersphere1)
	{
		return !operator<(hypersphere1, hypersphere0);
	}

	template <typename T, size_t N>
	bool operator>(Hypersphere<T, N> const& hypersphere0, Hypersphere<T, N> const& hypersphere1)
	{
		return operator<(hypersphere1, hypersphere0);
	}

	template <typename T, size_t N>
	bool operator>=(Hypersphere<T, N> const& hypersphere0, Hypersphere<T, N> const& hypersphere1)
	{
		return !operator<(hypersphere0, hypersphere1);
	}

	// Template aliases for convenience.
	template <typename T> using Circle2 = Hypersphere<T, 2>;
	template <typename T> using Sphere3 = Hypersphere<T, 3>;
}



