// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Math.h>
#include <vector>

namespace gte
{
    template <typename Real>
    class GVector
    {
    public:
        // The tuple is length zero (uninitialized).
        GVector()
            :
            mTuple{}
        {
        }

        // The tuple is length 'size' and the elements are uninitialized.
        GVector(int32_t size)
            :
            mTuple{}
        {
            SetSize(size);
        }

        // For 0 <= d <= size, element d is 1 and all others are zero.  If d
        // is invalid, the zero vector is created.  This is a convenience for
        // creating the standard Euclidean basis vectors; see also
        // MakeUnit(int32_t,int32_t) and Unit(int32_t,int32_t).
        GVector(int32_t size, int32_t d)
            :
            mTuple{}
        {
            SetSize(size);
            MakeUnit(d);
        }

        // The copy constructor, destructor, and assignment operator are
        // generated by the compiler.

        // Member access.  SetSize(int32_t) does not initialize the tuple.  The
        // first operator[] returns a const reference rather than a Real
        // value.  This supports writing via standard file operations that
        // require a const pointer to data.
        void SetSize(int32_t size)
        {
            LogAssert(size >= 0, "Invalid size.");
            mTuple.resize(size);
        }

        inline int32_t GetSize() const
        {
            return static_cast<int32_t>(mTuple.size());
        }

        inline Real const& operator[](int32_t i) const
        {
            return mTuple[i];
        }

        inline Real& operator[](int32_t i)
        {
            return mTuple[i];
        }

        // Comparison (for use by STL containers).
        inline bool operator==(GVector const& vec) const
        {
            return mTuple == vec.mTuple;
        }

        inline bool operator!=(GVector const& vec) const
        {
            return mTuple != vec.mTuple;
        }

        inline bool operator< (GVector const& vec) const
        {
            return mTuple < vec.mTuple;
        }

        inline bool operator<=(GVector const& vec) const
        {
            return mTuple <= vec.mTuple;
        }

        inline bool operator> (GVector const& vec) const
        {
            return mTuple > vec.mTuple;
        }

        inline bool operator>=(GVector const& vec) const
        {
            return mTuple >= vec.mTuple;
        }

        // Special vectors.

        // All components are 0.
        void MakeZero()
        {
            std::fill(mTuple.begin(), mTuple.end(), (Real)0);
        }

        // Component d is 1, all others are zero.
        void MakeUnit(int32_t d)
        {
            std::fill(mTuple.begin(), mTuple.end(), (Real)0);
            if (0 <= d && d < (int32_t)mTuple.size())
            {
                mTuple[d] = (Real)1;
            }
        }

        static GVector Zero(int32_t size)
        {
            GVector<Real> v(size);
            v.MakeZero();
            return v;
        }

        static GVector Unit(int32_t size, int32_t d)
        {
            GVector<Real> v(size);
            v.MakeUnit(d);
            return v;
        }

    protected:
        // This data structure takes advantage of the built-in operator[],
        // range checking and visualizers in MSVS.
        std::vector<Real> mTuple;
    };

    // Unary operations.
    template <typename Real>
    GVector<Real> operator+(GVector<Real> const& v)
    {
        return v;
    }

    template <typename Real>
    GVector<Real> operator-(GVector<Real> const& v)
    {
        GVector<Real> result(v.GetSize());
        for (int32_t i = 0; i < v.GetSize(); ++i)
        {
            result[i] = -v[i];
        }
        return result;
    }

    // Linear-algebraic operations.
    template <typename Real>
    GVector<Real> operator+(GVector<Real> const& v0, GVector<Real> const& v1)
    {
        GVector<Real> result = v0;
        return result += v1;
    }

    template <typename Real>
    GVector<Real> operator-(GVector<Real> const& v0, GVector<Real> const& v1)
    {
        GVector<Real> result = v0;
        return result -= v1;
    }

    template <typename Real>
    GVector<Real> operator*(GVector<Real> const& v, Real scalar)
    {
        GVector<Real> result = v;
        return result *= scalar;
    }

    template <typename Real>
    GVector<Real> operator*(Real scalar, GVector<Real> const& v)
    {
        GVector<Real> result = v;
        return result *= scalar;
    }

    template <typename Real>
    GVector<Real> operator/(GVector<Real> const& v, Real scalar)
    {
        GVector<Real> result = v;
        return result /= scalar;
    }

    template <typename Real>
    GVector<Real>& operator+=(GVector<Real>& v0, GVector<Real> const& v1)
    {
        if (v0.GetSize() == v1.GetSize())
        {
            for (int32_t i = 0; i < v0.GetSize(); ++i)
            {
                v0[i] += v1[i];
            }
            return v0;
        }
        LogError("Mismatched sizes.");
    }

    template <typename Real>
    GVector<Real>& operator-=(GVector<Real>& v0, GVector<Real> const& v1)
    {
        if (v0.GetSize() == v1.GetSize())
        {
            for (int32_t i = 0; i < v0.GetSize(); ++i)
            {
                v0[i] -= v1[i];
            }
            return v0;
        }
        LogError("Mismatched sizes.");
    }

    template <typename Real>
    GVector<Real>& operator*=(GVector<Real>& v, Real scalar)
    {
        for (int32_t i = 0; i < v.GetSize(); ++i)
        {
            v[i] *= scalar;
        }
        return v;
    }

    template <typename Real>
    GVector<Real>& operator/=(GVector<Real>& v, Real scalar)
    {
        if (scalar != (Real)0)
        {
            Real invScalar = (Real)1 / scalar;
            for (int32_t i = 0; i < v.GetSize(); ++i)
            {
                v[i] *= invScalar;
            }
            return v;
        }
        LogError("Division by zero.");
    }

    // Geometric operations.  The functions with 'robust' set to 'false' use
    // the standard algorithm for normalizing a vector by computing the length
    // as a square root of the squared length and dividing by it.  The results
    // can be infinite (or NaN) if the length is zero.  When 'robust' is set
    // to 'true', the algorithm is designed to avoid floating-point overflow
    // and sets the normalized vector to zero when the length is zero.
    template <typename Real>
    Real Dot(GVector<Real> const& v0, GVector<Real> const& v1)
    {
        if (v0.GetSize() == v1.GetSize())
        {
            Real dot(0);
            for (int32_t i = 0; i < v0.GetSize(); ++i)
            {
                dot += v0[i] * v1[i];
            }
            return dot;
        }
        LogError("Mismatched sizes.");
    }

    template <typename Real>
    Real Length(GVector<Real> const& v, bool robust = false)
    {
        if (robust)
        {
            Real maxAbsComp = std::fabs(v[0]);
            for (int32_t i = 1; i < v.GetSize(); ++i)
            {
                Real absComp = std::fabs(v[i]);
                if (absComp > maxAbsComp)
                {
                    maxAbsComp = absComp;
                }
            }

            Real length;
            if (maxAbsComp > (Real)0)
            {
                GVector<Real> scaled = v / maxAbsComp;
                length = maxAbsComp * std::sqrt(Dot(scaled, scaled));
            }
            else
            {
                length = (Real)0;
            }
            return length;
        }
        else
        {
            return std::sqrt(Dot(v, v));
        }
    }

    template <typename Real>
    Real Normalize(GVector<Real>& v, bool robust = false)
    {
        if (robust)
        {
            Real maxAbsComp = std::fabs(v[0]);
            for (int32_t i = 1; i < v.GetSize(); ++i)
            {
                Real absComp = std::fabs(v[i]);
                if (absComp > maxAbsComp)
                {
                    maxAbsComp = absComp;
                }
            }

            Real length;
            if (maxAbsComp > (Real)0)
            {
                v /= maxAbsComp;
                length = std::sqrt(Dot(v, v));
                v /= length;
                length *= maxAbsComp;
            }
            else
            {
                length = (Real)0;
                for (int32_t i = 0; i < v.GetSize(); ++i)
                {
                    v[i] = (Real)0;
                }
            }
            return length;
        }
        else
        {
            Real length = std::sqrt(Dot(v, v));
            if (length > (Real)0)
            {
                v /= length;
            }
            else
            {
                for (int32_t i = 0; i < v.GetSize(); ++i)
                {
                    v[i] = (Real)0;
                }
            }
            return length;
        }
    }

    // Gram-Schmidt orthonormalization to generate orthonormal vectors from
    // the linearly independent inputs.  The function returns the smallest
    // length of the unnormalized vectors computed during the process.  If
    // this value is nearly zero, it is possible that the inputs are linearly
    // dependent (within numerical round-off errors).  On input,
    // 1 <= numElements <= N and v[0] through v[numElements-1] must be
    // initialized.  On output, the vectors v[0] through v[numElements-1]
    // form an orthonormal set.
    template <typename Real>
    Real Orthonormalize(int32_t numInputs, GVector<Real>* v, bool robust = false)
    {
        if (v && 1 <= numInputs && numInputs <= v[0].GetSize())
        {
            for (int32_t i = 1; i < numInputs; ++i)
            {
                if (v[0].GetSize() != v[i].GetSize())
                {
                    LogError("Mismatched sizes.");
                }
            }

            Real minLength = Normalize(v[0], robust);
            for (int32_t i = 1; i < numInputs; ++i)
            {
                for (int32_t j = 0; j < i; ++j)
                {
                    Real dot = Dot(v[i], v[j]);
                    v[i] -= v[j] * dot;
                }
                Real length = Normalize(v[i], robust);
                if (length < minLength)
                {
                    minLength = length;
                }
            }
            return minLength;
        }
        LogError("Invalid input.");
    }

    // Compute the axis-aligned bounding box of the vectors.  The return value is
    // 'true' iff the inputs are valid, in which case vmin and vmax have valid
    // values.
    template <typename Real>
    bool ComputeExtremes(int32_t numVectors, GVector<Real> const* v,
        GVector<Real>& vmin, GVector<Real>& vmax)
    {
        if (v && numVectors > 0)
        {
            for (int32_t i = 1; i < numVectors; ++i)
            {
                if (v[0].GetSize() != v[i].GetSize())
                {
                    LogError("Mismatched sizes.");
                }
            }

            int32_t const size = v[0].GetSize();
            vmin = v[0];
            vmax = vmin;
            for (int32_t j = 1; j < numVectors; ++j)
            {
                GVector<Real> const& vec = v[j];
                for (int32_t i = 0; i < size; ++i)
                {
                    if (vec[i] < vmin[i])
                    {
                        vmin[i] = vec[i];
                    }
                    else if (vec[i] > vmax[i])
                    {
                        vmax[i] = vec[i];
                    }
                }
            }
            return true;
        }
        LogError("Invalid input.");
    }

    // Lift n-tuple v to homogeneous (n+1)-tuple (v,last).
    template <typename Real>
    GVector<Real> HLift(GVector<Real> const& v, Real last)
    {
        int32_t const size = v.GetSize();
        GVector<Real> result(size + 1);
        for (int32_t i = 0; i < size; ++i)
        {
            result[i] = v[i];
        }
        result[size] = last;
        return result;
    }

    // Project homogeneous n-tuple v = (u,v[n-1]) to (n-1)-tuple u.
    template <typename Real>
    GVector<Real> HProject(GVector<Real> const& v)
    {
        int32_t const size = v.GetSize();
        if (size > 1)
        {
            GVector<Real> result(size - 1);
            for (int32_t i = 0; i < size - 1; ++i)
            {
                result[i] = v[i];
            }
            return result;
        }
        else
        {
            return GVector<Real>();
        }
    }


    // Lift n-tuple v = (w0,w1) to (n+1)-tuple u = (w0,u[inject],w1).  By
    // inference, w0 is a (inject)-tuple [nonexistent when inject=0] and w1
    // is a (n-inject)-tuple [nonexistent when inject=n].
    template <typename Real>
    GVector<Real> Lift(GVector<Real> const& v, int32_t inject, Real value)
    {
        int32_t const size = v.GetSize();
        GVector<Real> result(size + 1);
        int32_t i;
        for (i = 0; i < inject; ++i)
        {
            result[i] = v[i];
        }
        result[i] = value;
        int32_t j = i;
        for (++j; i < size; ++i, ++j)
        {
            result[j] = v[i];
        }
        return result;
    }

    // Project n-tuple v = (w0,v[reject],w1) to (n-1)-tuple u = (w0,w1).  By
    // inference, w0 is a (reject)-tuple [nonexistent when reject=0] and w1
    // is a (n-1-reject)-tuple [nonexistent when reject=n-1].
    template <typename Real>
    GVector<Real> Project(GVector<Real> const& v, int32_t reject)
    {
        int32_t const size = v.GetSize();
        if (size > 1)
        {
            GVector<Real> result(size - 1);
            for (int32_t i = 0, j = 0; i < size - 1; ++i, ++j)
            {
                if (j == reject)
                {
                    ++j;
                }
                result[i] = v[j];
            }
            return result;
        }
        else
        {
            return GVector<Real>();
        }
    }
}
