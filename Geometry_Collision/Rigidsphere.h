#include "RigidBody.h"
#include "Hypersphere.h"
using namespace Vector_GM;

class RigidSphere : public RigidBody<double>
{
public:
	RigidSphere(Sphere3<double> const& sphere, double massDensity);
	virtual ~RigidSphere() = default;

	inline Sphere3<double> const& GetWorldSphere() const
	{
		return mWorldSphere;
	}

	inline double GetRadius() const
	{
		return mWorldSphere.radius;
	}

	void UpdateWorldQuantities();

private:
	Sphere3<double> mWorldSphere;
};
