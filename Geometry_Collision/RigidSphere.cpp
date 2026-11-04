// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06
//
// Copied from GTE 6.x sample Physics/BouncingSpheres/RigidSphere.cpp
// and modified for Geometry_Collision: namespace gte renamed to
// Vector_GM, includes point to the copies in this folder. The body
// inertia tensor is corrected to (2/5)*m*r^2*I (see the constructor).

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
	// Inertia tensor of a solid sphere about its center: (2/5)*m*r^2*I.
	// SetBodyInertia expects the inertia tensor itself (it computes the
	// inverse). The GTE 6.x sample passed massDensity * I, which has the
	// units of a density, not of a moment of inertia.
	double momentOfInertia = 0.4 * mass * sphere.radius * sphere.radius;
	Matrix3x3<double> bodyInertia = momentOfInertia * Matrix3x3<double>::Identity();
	SetMass(mass);
	SetBodyInertia(bodyInertia);
	SetPosition(sphere.center);
	UpdateWorldQuantities();
}

void RigidSphere::UpdateWorldQuantities()
{
	mWorldSphere.center = GetPosition();
}
