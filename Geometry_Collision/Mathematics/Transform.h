// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include "Logger.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include "Rotation.h"

// Transforms when GTE_USE_MAT_VEC is defined in the preprocessor symbols.
//
// The transform is Y = M*X+T, where M is a 3-by-3 matrix and T is a 3x1
// translation.  In most cases, M = R, a rotation matrix, or M = R*S,
// where R is a rotation matrix and S is a diagonal matrix whose diagonal
// entries are positive scales.  To support modeling packages that allow
// general affine transforms, M can be any invertible 3x3 matrix.  The vector
// X is transformed in the "forward" direction to Y.  The "inverse" direction
// transforms Y to X, namely X = M^{-1}*(Y-T) in the general case.  In the
// special case of M = R*S, the inverse direction is X = S^{-1}*R^t*(Y-T),
// where S^{-1} is the diagonal matrix whose diagonal entries are the
// reciprocoals of those of S and where R^t is the transpose of R.  For SIMD
// support of matrix-vector and matrix-matrix multiplications, a homogeneous
// matrix H = {{M,T},{0,1}} is stored by this class.  The forward transform is
// {Y,1} = H*{X,1} and the inverse transform is {X,1} = H^{-1}*{Y,1}.

// Transforms when GTE_USE_MAT_VEC is NOT defined in the preprocessor symbols.
//
// The transform is Y = T + X*M, where M is a 3-by-3 matrix and T is a 1x3
// translation.  In most cases, M = R, a rotation matrix, or M = S*R,
// where R is a rotation matrix and S is a diagonal matrix whose diagonal
// entries are positive scales.  To support modeling packages that allow
// general affine transforms, M can be any invertible 3x3 matrix.  The vector
// X is transformed in the "forward" direction to Y.  The "inverse" direction
// transforms Y to X, namely X = (Y-T)*M^{-1} in the general case.  In the
// special case of M = S*R, the inverse direction is X = (Y-T)*R^t*S^{-1},
// where S^{-1} is the diagonal matrix whose diagonal entries are the
// reciprocoals of those of S and where R^t is the transpose of R.  For SIMD
// support of matrix-vector and matrix-matrix multiplications, a homogeneous
// matrix H = {{M,0},{T,1}} is stored by this class.  The forward transform is
// {Y,1} = {X,1}*H and the inverse transform is {X,1} = {Y,1}*H^{-1}.

// With either multiplication convention, a matrix M = R*S (GTE_USE_MAT_VEC)
// or a matrix M = S*R (!GTE_USE_VEC_MAT) is referred to as an "RS-matrix".
// The class does not provide a member function to compute the inverse of a
// transform:  'Transform GetInverse() const'.  If one were to add this,
// be aware that the inverse of an RS-matrix is not generally an RS-matrix;
// that is, the inverse of R*S is S^{-1}*R^t which cannot always be factored
// as S^{-1} * R^t = R' * S'.  You would need to SetMatrix using S^{-1}*R^t
// as the input.

namespace Vector_GM
{
    template <typename Real>
    class Transform
    {
    public:
        // The default constructor produces the identity transformation.  The
        // default copy constructor and assignment operator are generated by
        // the compiler.
        Transform()
            :
            mHMatrix{},
            mInvHMatrix{},
            mMatrix{},
            mTranslate{ (Real)0, (Real)0, (Real)0, (Real)1 },
            mScale{ (Real)1, (Real)1, (Real)1, (Real)1 },
            mIsIdentity(true),
            mIsRSMatrix(true),
            mIsUniformScale(true),
            mInverseNeedsUpdate(false)
        {
            mHMatrix.MakeIdentity();
            mInvHMatrix.MakeIdentity();
            mMatrix.MakeIdentity();
        }

        // Implicit conversion.
        inline operator Matrix4x4<Real> const& () const
        {
            return GetHMatrix();
        }

        // Set the transformation to the identity matrix.
        void MakeIdentity()
        {
            mMatrix.MakeIdentity();
            mTranslate = { (Real)0, (Real)0, (Real)0, (Real)1 };
            mScale = { (Real)1, (Real)1, (Real)1, (Real)1 };
            mIsIdentity = true;
            mIsRSMatrix = true;
            mIsUniformScale = true;
            UpdateHMatrix();
        }

        // Set the transformation to have scales of 1.
        void MakeUnitScale()
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            mScale = { (Real)1, (Real)1, (Real)1, (Real)1 };
            mIsUniformScale = true;
            UpdateHMatrix();
        }

        // Hints about the structure of the transformation.

        // M = I
        inline bool IsIdentity() const
        {
            return mIsIdentity;
        }

        // R*S (GTE_USE_MAT_VEC defined) or S*R (GTE_USE_MAT_VEC not defined)
        inline bool IsRSMatrix() const
        {
            return mIsRSMatrix;
        }

        // RS-matrix with S = c*I
        inline bool IsUniformScale() const
        {
            return mIsRSMatrix && mIsUniformScale;
        }

        // Member access.
        // (1) The Set* functions set the is-identity hint to false.
        // (2) The SetRotate function sets the is-rsmatrix hint to true.  If this
        //     hint is false,  GetRotate triggers an assertion in debug mode.
        // (3) The SetMatrix function sets the is-rsmatrix and is-uniform-scale
        //     hints to false.
        // (4) The SetScale function sets the is-uniform-scale hint to false.
        //     The SetUniformScale function sets the is-uniform-scale hint to
        //     true.  If this hint is false, GetUniformScale triggers an assertion
        //     in debug mode.
        // (5) All Set* functions set the inverse-needs-update to true.  When
        //     GetHInverse is called, the inverse must be computed in this case
        //     and the inverse-needs-update is reset to false.

        // {{R,0},{0,1}}
        void SetRotation(Matrix4x4<Real> const& rotate)
        {
            mMatrix = rotate;
            mIsIdentity = false;
            mIsRSMatrix = true;
            UpdateHMatrix();
        }

        // {{M,0},{0,1}}
        void SetMatrix(Matrix4x4<Real> const& matrix)
        {
            mMatrix = matrix;
            mIsIdentity = false;
            mIsRSMatrix = false;
            mIsUniformScale = false;
            UpdateHMatrix();
        }

        void SetTranslation(Real x0, Real x1, Real x2)
        {
            mTranslate = Vector4<Real>{ x0, x1, x2, (Real)1 };
            mIsIdentity = false;
            UpdateHMatrix();
        }

        inline void SetTranslation(Vector3<Real> const& translate)
        {
            SetTranslation(translate[0], translate[1], translate[2]);
        }

        inline void SetTranslation(Vector4<Real> const& translate)
        {
            SetTranslation(translate[0], translate[1], translate[2]);
        }

        void SetScale(Real s0, Real s1, Real s2)
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            LogAssert(s0 != (Real)0 && s1 != (Real)0 && s2 != (Real)0, "Scales must be nonzero.");
            mScale = { s0, s1, s2, (Real)1 };
            mIsIdentity = false;
            mIsUniformScale = false;
            UpdateHMatrix();
        }

        inline void SetScale(Vector3<Real> const& scale)
        {
            SetScale(scale[0], scale[1], scale[2]);
        }

        inline void SetScale(Vector4<Real> const& scale)
        {
            SetScale(scale[0], scale[1], scale[2]);
        }

        void SetUniformScale(Real scale)
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            LogAssert(scale != (Real)0, "Scale must be nonzero.");
            mScale = { scale, scale, scale, (Real)1 };
            mIsIdentity = false;
            mIsUniformScale = true;
            UpdateHMatrix();
        }

        // {{R,0},{0,1}}
        Matrix4x4<Real> const& GetRotation() const
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            return mMatrix;
        }

        // {{M,0},{0,1}}
        inline Matrix4x4<Real> const& GetMatrix() const
        {
            return mMatrix;
        }

        // (x,y,z)
        inline Vector3<Real> GetTranslation() const
        {
            return Vector3<Real>{ mTranslate[0], mTranslate[1], mTranslate[2] };
        }

        // (x,y,z,0)
        inline Vector4<Real> GetTranslationW0() const
        {
            return Vector4<Real>{ mTranslate[0], mTranslate[1], mTranslate[2], (Real)0 };
        }

        // (x,y,z,1)
        inline Vector4<Real> GetTranslationW1() const
        {
            return Vector4<Real>{ mTranslate[0], mTranslate[1], mTranslate[2], (Real)1 };
        }

        // (s0,s1,s2)
        Vector3<Real> GetScale() const
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            return Vector3<Real>{ mScale[0], mScale[1], mScale[2] };
        }

        // (s0,s1,s2,1)
        Vector4<Real> GetScaleW1() const
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            return Vector4<Real>{ mScale[0], mScale[1], mScale[2], (Real)1 };
        }

        Real GetUniformScale() const
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            LogAssert(mIsUniformScale, "Transform is not uniform scale.");
            return mScale[0];
        }

        // Alternate representations to set/get the rotation.

        // Set/get from 3x3 matrices.
        void SetRotation(Matrix3x3<Real> const& rotate)
        {
            mMatrix.MakeIdentity();
            for (int32_t r = 0; r < 3; ++r)
            {
                for (int32_t c = 0; c < 3; ++c)
                {
                    mMatrix(r, c) = rotate(r, c);
                }
            }

            mIsIdentity = false;
            mIsRSMatrix = true;
            UpdateHMatrix();
        }

        void GetRotation(Matrix3x3<Real>& rotate) const
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            for (int32_t r = 0; r < 3; ++r)
            {
                for (int32_t c = 0; c < 3; ++c)
                {
                    rotate(r, c) = mMatrix(r, c);
                }
            }
        }

        // The quaternion is unit length.
        void SetRotation(Quaternion<Real> const& q)
        {
            mMatrix = Rotation<4, Real>(q);
            mIsIdentity = false;
            mIsRSMatrix = true;
            UpdateHMatrix();
        }

        void GetRotation(Quaternion<Real>& q) const
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            q = Rotation<4, Real>(mMatrix);
        }

        // The axis is unit length and the angle is in radians.
        void SetRotation(AxisAngle<3, Real> const& axisAngle)
        {
            AxisAngle<4, Real> aa4(HLift(axisAngle.axis, (Real)1), axisAngle.angle);
            mMatrix = Rotation<4, Real>(aa4);
            mIsIdentity = false;
            mIsRSMatrix = true;
            UpdateHMatrix();
        }

        void GetRotation(AxisAngle<3, Real>& axisAngle) const
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            AxisAngle<4, Real> aa4 = Rotation<4, Real>(mMatrix);
            axisAngle.axis = HProject(aa4.axis);
            axisAngle.angle = aa4.angle;
        }

        void SetRotation(AxisAngle<4, Real> const& axisAngle)
        {
            mMatrix = Rotation<4, Real>(axisAngle);
            mIsIdentity = false;
            mIsRSMatrix = true;
            UpdateHMatrix();
        }

        void GetRotation(AxisAngle<4, Real>& axisAngle) const
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            axisAngle = Rotation<4, Real>(mMatrix);
        }

        // The Euler angles are in radians.  The GetEulerAngles function
        // expects the eulerAngles.axis[] values to be set to the axis order
        // you want.
        void SetRotation(EulerAngles<Real> const& eulerAngles)
        {
            mMatrix = Rotation<4, Real>(eulerAngles);
            mIsIdentity = false;
            mIsRSMatrix = true;
            UpdateHMatrix();
        }

        void GetRotation(EulerAngles<Real>& eulerAngles) const
        {
            LogAssert(mIsRSMatrix, "Transform is not rotation-scale.");
            eulerAngles = Rotation<4, Real>(mMatrix)(eulerAngles.axis[0],
                eulerAngles.axis[1], eulerAngles.axis[2]);
        }

        // For M = R*S or M = S*R, the largest value of S in absolute value is
        // returned. For general M, the max-row-sum norm is returned when
        // GTE_USE_MAT_VEC is defined or the max-col-sum norm is returned when
        // the GTE_USE_MAT_VEC is not defined, which is a reasonable measure
        // of maximum scale of the transformation.
        Real GetNorm() const
        {
            Real sum0, sum1, sum2;

            if (mIsRSMatrix)
            {
                // A RS matrix (GTE_USE_MAT_VEC defined) or an SR matrix 
                // (GTE_USE_MAT_VEC is not defined).
                sum0 = std::fabs(mScale[0]);
                sum1 = std::fabs(mScale[1]);
                sum2 = std::fabs(mScale[2]);
            }
            else
            {
                // The spectral norm (the maximum absolute value of the
                // eigenvalues) is smaller or equal to this norm.  Therefore,
                // this function returns an approximation to the maximum
                // scale.

#if defined(GTE_USE_MAT_VEC)
                // Use the max-row-sum matrix norm.
                sum0 = std::fabs(mMatrix(0, 0)) + std::fabs(mMatrix(0, 1)) + std::fabs(mMatrix(0, 2));
                sum1 = std::fabs(mMatrix(1, 0)) + std::fabs(mMatrix(1, 1)) + std::fabs(mMatrix(1, 2));
                sum2 = std::fabs(mMatrix(2, 0)) + std::fabs(mMatrix(2, 1)) + std::fabs(mMatrix(2, 2));
#else
                // Use the max-col-sum matrix norm.
                sum0 = std::fabs(mMatrix(0, 0)) + std::fabs(mMatrix(1, 0)) + std::fabs(mMatrix(2, 0));
                sum1 = std::fabs(mMatrix(0, 1)) + std::fabs(mMatrix(1, 1)) + std::fabs(mMatrix(2, 1));
                sum2 = std::fabs(mMatrix(0, 2)) + std::fabs(mMatrix(1, 2)) + std::fabs(mMatrix(2, 2));
#endif
            }

            return std::max(std::max(sum0, sum1), sum2);
        }

        // Get the homogeneous matrix (composite of all channels).
        inline Matrix4x4<Real> const& GetHMatrix() const
        {
            return mHMatrix;
        }

        // Get the inverse homogeneous matrix, recomputing it when necessary.
        // GTE_USE_MAT_VEC
        //     H = {{M,T},{0,1}}, then H^{-1} = {{M^{-1},-M^{-1}*T},{0,1}}
        // GTE_USE_VEC_MAT
        //     H = {{M,0},{T,1}}, then H^{-1} = {{M^{-1},0},{-M^{-1}*T,1}}
        Matrix4x4<Real> const& GetHInverse() const
        {
            if (mInverseNeedsUpdate)
            {
                if (mIsIdentity)
                {
                    mInvHMatrix.MakeIdentity();
                }
                else
                {
                    if (mIsRSMatrix)
                    {
                        if (mIsUniformScale)
                        {
                            Real invScale = (Real)1 / mScale[0];
#if defined(GTE_USE_MAT_VEC)
                            mInvHMatrix(0, 0) = invScale * mMatrix(0, 0);
                            mInvHMatrix(0, 1) = invScale * mMatrix(1, 0);
                            mInvHMatrix(0, 2) = invScale * mMatrix(2, 0);
                            mInvHMatrix(1, 0) = invScale * mMatrix(0, 1);
                            mInvHMatrix(1, 1) = invScale * mMatrix(1, 1);
                            mInvHMatrix(1, 2) = invScale * mMatrix(2, 1);
                            mInvHMatrix(2, 0) = invScale * mMatrix(0, 2);
                            mInvHMatrix(2, 1) = invScale * mMatrix(1, 2);
                            mInvHMatrix(2, 2) = invScale * mMatrix(2, 2);
#else
                            mInvHMatrix(0, 0) = mMatrix(0, 0) * invScale;
                            mInvHMatrix(0, 1) = mMatrix(1, 0) * invScale;
                            mInvHMatrix(0, 2) = mMatrix(2, 0) * invScale;
                            mInvHMatrix(1, 0) = mMatrix(0, 1) * invScale;
                            mInvHMatrix(1, 1) = mMatrix(1, 1) * invScale;
                            mInvHMatrix(1, 2) = mMatrix(2, 1) * invScale;
                            mInvHMatrix(2, 0) = mMatrix(0, 2) * invScale;
                            mInvHMatrix(2, 1) = mMatrix(1, 2) * invScale;
                            mInvHMatrix(2, 2) = mMatrix(2, 2) * invScale;
#endif
                        }
                        else
                        {
                            // Replace 3 reciprocals by 6 multiplies and
                            // 1 reciprocal.
                            Real s01 = mScale[0] * mScale[1];
                            Real s02 = mScale[0] * mScale[2];
                            Real s12 = mScale[1] * mScale[2];
                            Real invs012 = (Real)1 / (s01 * mScale[2]);
                            Real invS0 = s12 * invs012;
                            Real invS1 = s02 * invs012;
                            Real invS2 = s01 * invs012;
#if defined(GTE_USE_MAT_VEC)
                            mInvHMatrix(0, 0) = invS0 * mMatrix(0, 0);
                            mInvHMatrix(0, 1) = invS0 * mMatrix(1, 0);
                            mInvHMatrix(0, 2) = invS0 * mMatrix(2, 0);
                            mInvHMatrix(1, 0) = invS1 * mMatrix(0, 1);
                            mInvHMatrix(1, 1) = invS1 * mMatrix(1, 1);
                            mInvHMatrix(1, 2) = invS1 * mMatrix(2, 1);
                            mInvHMatrix(2, 0) = invS2 * mMatrix(0, 2);
                            mInvHMatrix(2, 1) = invS2 * mMatrix(1, 2);
                            mInvHMatrix(2, 2) = invS2 * mMatrix(2, 2);
#else
                            mInvHMatrix(0, 0) = mMatrix(0, 0) * invS0;
                            mInvHMatrix(0, 1) = mMatrix(1, 0) * invS1;
                            mInvHMatrix(0, 2) = mMatrix(2, 0) * invS2;
                            mInvHMatrix(1, 0) = mMatrix(0, 1) * invS0;
                            mInvHMatrix(1, 1) = mMatrix(1, 1) * invS1;
                            mInvHMatrix(1, 2) = mMatrix(2, 1) * invS2;
                            mInvHMatrix(2, 0) = mMatrix(0, 2) * invS0;
                            mInvHMatrix(2, 1) = mMatrix(1, 2) * invS1;
                            mInvHMatrix(2, 2) = mMatrix(2, 2) * invS2;
#endif
                        }
                    }
                    else
                    {
                        mInvHMatrix = gte::Inverse(mHMatrix);
                    }

#if defined(GTE_USE_MAT_VEC)
                    mInvHMatrix(0, 3) = -(
                        mInvHMatrix(0, 0) * mTranslate[0] +
                        mInvHMatrix(0, 1) * mTranslate[1] +
                        mInvHMatrix(0, 2) * mTranslate[2]
                        );

                    mInvHMatrix(1, 3) = -(
                        mInvHMatrix(1, 0) * mTranslate[0] +
                        mInvHMatrix(1, 1) * mTranslate[1] +
                        mInvHMatrix(1, 2) * mTranslate[2]
                        );

                    mInvHMatrix(2, 3) = -(
                        mInvHMatrix(2, 0) * mTranslate[0] +
                        mInvHMatrix(2, 1) * mTranslate[1] +
                        mInvHMatrix(2, 2) * mTranslate[2]
                        );

                    // The last row of mHMatrix is always (0,0,0,1) for an
                    // affine transformation, so it is set once in the
                    // constructor.  It is not necessary to reset it here.
#else
                    mInvHMatrix(3, 0) = -(
                        mInvHMatrix(0, 0) * mTranslate[0] +
                        mInvHMatrix(1, 0) * mTranslate[1] +
                        mInvHMatrix(2, 0) * mTranslate[2]
                        );

                    mInvHMatrix(3, 1) = -(
                        mInvHMatrix(0, 1) * mTranslate[0] +
                        mInvHMatrix(1, 1) * mTranslate[1] +
                        mInvHMatrix(2, 1) * mTranslate[2]
                        );

                    mInvHMatrix(3, 2) = -(
                        mInvHMatrix(0, 2) * mTranslate[0] +
                        mInvHMatrix(1, 2) * mTranslate[1] +
                        mInvHMatrix(2, 2) * mTranslate[2]
                        );

                    // The last column of mHMatrix is always (0,0,0,1) for an
                    // affine transformation, so it is set once in the
                    // constructor.  It is not necessary to reset it here.
#endif
                }

                mInverseNeedsUpdate = false;
            }

            return mInvHMatrix;
        }

        // Invert the transform.  If possible, the channels are properly
        // assigned.  For example, if the input has mIsRSMatrix equal to
        // 'true', then the inverse also has mIsRSMatrix equal to 'true'
        // and the inverse's mMatrix is a rotation matrix and mScale is
        // set accordingly.
        Transform Inverse() const
        {
            Transform inverse{};  // = the identity

            if (!mIsIdentity)
            {
                if (mIsRSMatrix && mIsUniformScale)
                {
                    Matrix4x4<Real> invRotate = Transpose(GetRotation());
                    Real invScale = static_cast<Real>(1) / GetUniformScale();
                    Vector4<Real> invTranslate = -invScale * (invRotate * GetTranslationW1());
                    inverse.SetRotation(invRotate);
                    inverse.SetUniformScale(invScale);
                    inverse.SetTranslation(invTranslate);
                }
                else
                {
                    Matrix4x4<Real> invMatrix = gte::Inverse(GetHMatrix());
                    Vector4<Real> invTranslate = invMatrix.GetCol(3);
                    inverse.SetMatrix(invMatrix);
                    inverse.SetTranslation(invTranslate);
                }
            }

            return inverse;
        }

        // The identity transformation.
        static Transform Identity()
        {
            static Transform identity;
            return identity;
        }

    private:
        // Fill in the entries of mHMatrix whenever one of the components
        // mMatrix, mTranslate, or mScale changes.
        void UpdateHMatrix()
        {
            if (mIsIdentity)
            {
                mHMatrix.MakeIdentity();
            }
            else
            {
                if (mIsRSMatrix)
                {
#if defined(GTE_USE_MAT_VEC)
                    mHMatrix(0, 0) = mMatrix(0, 0) * mScale[0];
                    mHMatrix(0, 1) = mMatrix(0, 1) * mScale[1];
                    mHMatrix(0, 2) = mMatrix(0, 2) * mScale[2];
                    mHMatrix(1, 0) = mMatrix(1, 0) * mScale[0];
                    mHMatrix(1, 1) = mMatrix(1, 1) * mScale[1];
                    mHMatrix(1, 2) = mMatrix(1, 2) * mScale[2];
                    mHMatrix(2, 0) = mMatrix(2, 0) * mScale[0];
                    mHMatrix(2, 1) = mMatrix(2, 1) * mScale[1];
                    mHMatrix(2, 2) = mMatrix(2, 2) * mScale[2];
#else
                    mHMatrix(0, 0) = mScale[0] * mMatrix(0, 0);
                    mHMatrix(0, 1) = mScale[0] * mMatrix(0, 1);
                    mHMatrix(0, 2) = mScale[0] * mMatrix(0, 2);
                    mHMatrix(1, 0) = mScale[1] * mMatrix(1, 0);
                    mHMatrix(1, 1) = mScale[1] * mMatrix(1, 1);
                    mHMatrix(1, 2) = mScale[1] * mMatrix(1, 2);
                    mHMatrix(2, 0) = mScale[2] * mMatrix(2, 0);
                    mHMatrix(2, 1) = mScale[2] * mMatrix(2, 1);
                    mHMatrix(2, 2) = mScale[2] * mMatrix(2, 2);
#endif
                }
                else
                {
                    mHMatrix(0, 0) = mMatrix(0, 0);
                    mHMatrix(0, 1) = mMatrix(0, 1);
                    mHMatrix(0, 2) = mMatrix(0, 2);
                    mHMatrix(1, 0) = mMatrix(1, 0);
                    mHMatrix(1, 1) = mMatrix(1, 1);
                    mHMatrix(1, 2) = mMatrix(1, 2);
                    mHMatrix(2, 0) = mMatrix(2, 0);
                    mHMatrix(2, 1) = mMatrix(2, 1);
                    mHMatrix(2, 2) = mMatrix(2, 2);
                }

#if defined(GTE_USE_MAT_VEC)
                mHMatrix(0, 3) = mTranslate[0];
                mHMatrix(1, 3) = mTranslate[1];
                mHMatrix(2, 3) = mTranslate[2];

                // The last row of mHMatrix is always (0,0,0,1) for an affine
                // transformation, so it is set once in the constructor.  It
                // is not necessary to reset it here.
#else
                mHMatrix(3, 0) = mTranslate[0];
                mHMatrix(3, 1) = mTranslate[1];
                mHMatrix(3, 2) = mTranslate[2];

                // The last column of mHMatrix is always (0,0,0,1) for an
                // affine transformation, so it is set once in the
                // constructor.  It is not necessary to reset it here.
#endif
            }

            mInverseNeedsUpdate = true;
        }

        // Invert the 3x3 upper-left block of the input matrix.
        static void Invert3x3(Matrix4x4<Real> const& mat, Matrix4x4<Real>& invMat)
        {
            // Compute the adjoint of M (3x3).
            invMat(0, 0) = mat(1, 1) * mat(2, 2) - mat(1, 2) * mat(2, 1);
            invMat(0, 1) = mat(0, 2) * mat(2, 1) - mat(0, 1) * mat(2, 2);
            invMat(0, 2) = mat(0, 1) * mat(1, 2) - mat(0, 2) * mat(1, 1);
            invMat(0, 3) = 0.0f;
            invMat(1, 0) = mat(1, 2) * mat(2, 0) - mat(1, 0) * mat(2, 2);
            invMat(1, 1) = mat(0, 0) * mat(2, 2) - mat(0, 2) * mat(2, 0);
            invMat(1, 2) = mat(0, 2) * mat(1, 0) - mat(0, 0) * mat(1, 2);
            invMat(1, 3) = 0.0f;
            invMat(2, 0) = mat(1, 0) * mat(2, 1) - mat(1, 1) * mat(2, 0);
            invMat(2, 1) = mat(0, 1) * mat(2, 0) - mat(0, 0) * mat(2, 1);
            invMat(2, 2) = mat(0, 0) * mat(1, 1) - mat(0, 1) * mat(1, 0);
            invMat(2, 3) = 0.0f;
            invMat(3, 0) = 0.0f;
            invMat(3, 1) = 0.0f;
            invMat(3, 2) = 0.0f;
            invMat(3, 3) = 1.0f;

            // Compute the reciprocal of the determinant of M.
            Real invDet = (Real)1 / (
                mat(0, 0) * invMat(0, 0) +
                mat(0, 1) * invMat(1, 0) +
                mat(0, 2) * invMat(2, 0)
                );

            // inverse(M) = adjoint(M)/determinant(M).
            invMat(0, 0) *= invDet;
            invMat(0, 1) *= invDet;
            invMat(0, 2) *= invDet;
            invMat(1, 0) *= invDet;
            invMat(1, 1) *= invDet;
            invMat(1, 2) *= invDet;
            invMat(2, 0) *= invDet;
            invMat(2, 1) *= invDet;
            invMat(2, 2) *= invDet;
        }

        // The full 4x4 homogeneous matrix H and its inverse H^{-1}, stored
        // according to the conventions (see GetHInverse description).  The
        // inverse is computed only on demand.
        Matrix4x4<Real> mHMatrix;
        mutable Matrix4x4<Real> mInvHMatrix;

        Matrix4x4<Real> mMatrix;   // M (general) or R (rotation)
        Vector4<Real> mTranslate;  // T
        Vector4<Real> mScale;      // S
        bool mIsIdentity, mIsRSMatrix, mIsUniformScale;
        mutable bool mInverseNeedsUpdate;
    };

    // Compute M*V.
    template <typename Real>
    Vector4<Real> operator*(Transform<Real> const& M, Vector4<Real> const& V)
    {
        return M.GetHMatrix() * V;
    }

    // Compute V^T*M.
    template <typename Real>
    Vector4<Real> operator*(Vector4<Real> const& V, Transform<Real> const& M)
    {
        return V * M.GetHMatrix();
    }

    // Compute A*B.
    template <typename Real>
    Transform<Real> operator*(Transform<Real> const& A, Transform<Real> const& B)
    {
        if (A.IsIdentity())
        {
            return B;
        }

        if (B.IsIdentity())
        {
            return A;
        }

        Transform<Real> product;

        if (A.IsRSMatrix() && B.IsRSMatrix())
        {
#if defined(GTE_USE_MAT_VEC)
            if (A.IsUniformScale())
            {
                product.SetRotation(A.GetRotation() * B.GetRotation());

                product.SetTranslation(A.GetUniformScale() * (
                    A.GetRotation() * B.GetTranslationW0()) +
                    A.GetTranslationW1());

                if (B.IsUniformScale())
                {
                    product.SetUniformScale(A.GetUniformScale() * B.GetUniformScale());
                }
                else
                {
                    product.SetScale(A.GetUniformScale() * B.GetScale());
                }

                return product;
            }
#else
            if (B.IsUniformScale())
            {
                product.SetRotation(A.GetRotation() * B.GetRotation());

                product.SetTranslation(B.GetUniformScale() * (
                    A.GetTranslationW0() * B.GetRotation()) +
                    B.GetTranslationW1());

                if (A.IsUniformScale())
                {
                    product.SetUniformScale(A.GetUniformScale() * B.GetUniformScale());
                }
                else
                {
                    product.SetScale(A.GetScale() * B.GetUniformScale());
                }

                return product;
            }
#endif
        }

        // In all remaining cases, the matrix cannot be written as R*S*X+T.
        Matrix4x4<Real> matMA;
        if (A.IsRSMatrix())
        {
#if defined(GTE_USE_MAT_VEC)
            matMA = MultiplyMD(A.GetRotation(), A.GetScaleW1());
#else
            matMA = MultiplyDM(A.GetScaleW1(), A.GetRotation());
#endif
        }
        else
        {
            matMA = A.GetMatrix();
        }

        Matrix4x4<Real> matMB;
        if (B.IsRSMatrix())
        {
#if defined(GTE_USE_MAT_VEC)
            matMB = MultiplyMD(B.GetRotation(), B.GetScaleW1());
#else
            matMB = MultiplyDM(B.GetScaleW1(), B.GetRotation());
#endif
        }
        else
        {
            matMB = B.GetMatrix();
        }

        product.SetMatrix(matMA * matMB);
#if defined(GTE_USE_MAT_VEC)
        product.SetTranslation(matMA * B.GetTranslationW0() +
            A.GetTranslationW1());
#else
        product.SetTranslation(A.GetTranslationW0() * matMB +
            B.GetTranslationW1());
#endif
        return product;
    }

    template <typename Real>
    inline Matrix4x4<Real> operator*(Matrix4x4<Real> const& A, Transform<Real> const& B)
    {
        return A * B.GetHMatrix();
    }

    template <typename Real>
    inline Matrix4x4<Real> operator*(Transform<Real> const& A, Matrix4x4<Real> const& B)
    {
        return A.GetHMatrix()* B;
    }
}
