#include "RigidPlane.h"

RigidPlane::RigidPlane(Plane3<double> const& plane)
	:
	RigidBody<double>{},
	mPlane(plane)
{
	SetMass(0.0);
	SetBodyInertia(Matrix3x3<double>::Zero());
	SetPosition(mPlane.origin);
}
