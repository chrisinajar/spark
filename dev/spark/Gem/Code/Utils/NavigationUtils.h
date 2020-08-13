#pragma once

#include "SparkNavPath.h"
#include <AzCore/Component/EntityId.h>
#include <AzCore/Math/VectorConversions.h>

#define MOVEMENT_SCALE 0.035f
#define MOVEMENT_DISTANCE_THRESHOLD 0.1f
#define TURNRATE_SCALE 30.0f
#define MOVEMENT_ANGLE_DEADZONE 0.2007129f

namespace spark
{
	//N.B. all angles are in radiants
	
	extern float GetAngleFromDirection(AZ::Vector3 direction);
	extern AZ::Vector3 GetDirectionFromAngle(float angle);

	//return the minimum angle to add to alfa so that alfa is equal to beta
	// positive -> counterclockwise
	// negative -> clockwise
	extern float GetMinAngleDistance(float alfa, float beta);

	extern float GetMinAngleDistance(AZ::Vector3 a, AZ::Vector3 b);

	//make the angle be in the range [0,360)
	extern float NormalizeAngle(float angle);

	//distance ignoring z
	extern float Distance2D(AZ::Vector3 a, AZ::Vector3 b);
}
