// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06
//
// Copied from GTE 6.x sample Intersection/IntersectBoxSphere/IntersectBoxSphereWindow3.h
// and modified for Geometry_Collision: namespace gte renamed to
// Vector_GM, includes point to the copies in this folder.

#pragma once

#include <Applications/Window3.h>
#include <Graphics/ConstantColorEffect.h>
#include "IntrOrientedBoxShere.h"
using namespace gte;
using namespace Vector_GM;

class IntersectBoxSphereWindow3 : public Window3
{
public:
	IntersectBoxSphereWindow3(Parameters& parameters);

	virtual void OnIdle() override;
	virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
	void CreateScene();
	void Translate(int32_t direction, float delta);
	void Rotate(int32_t direction, float delta);
	void TestIntersection();

	std::shared_ptr<RasterizerState> mNoCullState;
	std::shared_ptr<RasterizerState> mNoCullWireState;
	std::shared_ptr<BlendState> mBlendState;
	std::shared_ptr<Visual> mSphereMesh, mBoxMesh;
	std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;
	Sphere3<float> mSphere;
	OrientedBox3<float> mBox;
	TIQuery<float, OrientedBox3<float>, Sphere3<float>> mQuery;
};
