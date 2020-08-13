#pragma once

#include <AzCore/std/string/string.h>
#include <AzCore/Component/EntityId.h>

namespace spark
{

	using TeamId = int;
	const TeamId InvalidTeamId = -1;


	using Number = float;

	#define SPARK_ENTITIES(o) \
		o(Unit)\
		o(Ability)\
		o(Item)\
		o(Modifier)\
		o(Shop)\
		o(ParticleSystem)


	#define o(n)  \
		using n##TypeId = AZStd::string;\
		using n##Id = AZ::EntityId;


    SPARK_ENTITIES(o)

	#undef o

/*  Equivalent to:


	using UnitTypeId = AZStd::string;
	using UnitId = AZ::EntityId;

	using AbilityTypeId = AZStd::string;
	using AbilityId = AZ::EntityId;

	...

*/
}