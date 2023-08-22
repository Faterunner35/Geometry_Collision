#include "RigidBody.h"
#include "Hyperplane.h"
using namespace Vector_GM;

class RigidPlane : public RigidBody<double>
{
public:
	RigidPlane(Plane3<double> const& plane);
	virtual ~RigidPlane() = default;

	inline Plane3<double> const& GetPlane() const
	{
		return mPlane;
	}

	inline double GetSignedDistance(Vector3<double> const& point) const
	{
		return Dot(mPlane.normal, point) - mPlane.constant;
	}

private:
	Plane3<double> mPlane;
};
