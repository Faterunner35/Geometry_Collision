#include "RigidSphere.h"
using namespace Vector_GM;

RigidSphere::RigidSphere(Sphere3<double> const& sphere, double massDensity)
	:
	RigidBody<double>{},
	mWorldSphere({ 0.0, 0.0, 0.0 }, sphere.radius)
{
	double rCubed = sphere.radius * sphere.radius * sphere.radius;
	double volume = 4.0 * GTE_C_PI * rCubed / 3.0;
	double mass = massDensity * volume;
	Matrix3x3<double> bodyInertia = massDensity * Matrix3x3<double>::Identity();
	SetMass(mass);
	SetBodyInertia(bodyInertia);
	SetPosition(sphere.center);
	UpdateWorldQuantities();
}

void RigidSphere::UpdateWorldQuantities()
{
	mWorldSphere.center = GetPosition();
}
