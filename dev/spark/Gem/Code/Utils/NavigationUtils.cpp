#include "spark_precompiled.h"

#include <MathConversion.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>


namespace spark
{

	using std::fabsf;
	float GetAngleFromDirection(AZ::Vector3 direction)
	{
		float directionAngle = std::atan2(direction.GetY(), direction.GetX());
		//directionAngle -= 0.5f*M_PI;//fix for the offset rotation of the model
		if (directionAngle < 0)directionAngle += 2 * M_PI;
		return directionAngle;
	}

	AZ::Vector3 GetDirectionFromAngle(float angle)
	{
		return AZ::Vector3(cos(angle), sin(angle),0.0f);
	}

	//return the minimum angle to add to alfa so that alfa is equal to beta
	// positive -> counterclockwise
	// negative -> clockwise
	float GetMinAngleDistance(float alfa, float beta)
	{
		float rx = alfa - beta;
		float lx = beta - alfa;

		if (rx < 0)rx += 2 * M_PI;
		if (lx <= 0)lx += 2 * M_PI;

		return (rx > lx ? +1 : -1) * min(rx, lx);
	}

	float GetMinAngleDistance(AZ::Vector3 a, AZ::Vector3 b)
	{
		a.Normalize();
		b.Normalize();

		return acosf(AZStd::clamp((float)a.Dot(b), 0.0f, 1.0f)); //need to clamp since the normalization is not exact -> dot product slighter bigger than 1 -> acosf arg out of domain -> Nan -> f*ck me
	}

	//make the angle be in the range [0,360)
	float NormalizeAngle(float angle)
	{
		angle = std::fmodf(angle, 2 * M_PI);
		if (angle < 0)angle += 2 * M_PI;

		return angle;
	}

	float Distance2D(AZ::Vector3 a, AZ::Vector3 b)
	{
		a.SetZ(0);
		b.SetZ(0);
		return a.GetDistance(b);
	}

}