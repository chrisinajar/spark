#include "spark_precompiled.h"

#include "sparkCVars.h"

#include <ISystem.h>
#include <IConsole.h>

namespace spark
{
	const char* SparkCVars::PLAYER_COUNT_CVAR = "sv_playerCount";

	SparkCVars::SparkCVars()
	{
	}

	SparkCVars::~SparkCVars()
	{
	}

	void SparkCVars::RegisterCVars()
	{
		REGISTER_INT(PLAYER_COUNT_CVAR, 1, VF_NULL, "Number of players to wait for before continuing on to game setup");
		
		REGISTER_INT("g_DebugTiles", 0, VF_NULL, "Enable/Disable tiles debug rendering\n0: disable\n1:enable");

		REGISTER_INT("g_FogOfWar", 1, VF_NULL, "Enable/Disable fog of war\n0: disable\n1:enable");
		REGISTER_FLOAT("g_FogOfWar_min_x", 0.0f, VF_NULL, "fogOfWar bounding box");
		REGISTER_FLOAT("g_FogOfWar_min_y", 0.0f, VF_NULL, "fogOfWar bounding box");
		REGISTER_FLOAT("g_FogOfWar_max_x", 0.0f, VF_NULL, "fogOfWar bounding box");
		REGISTER_FLOAT("g_FogOfWar_max_y", 0.0f, VF_NULL, "fogOfWar bounding box");
	}

	void SparkCVars::UnregisterCVars()
	{
		UNREGISTER_CVAR(PLAYER_COUNT_CVAR);

		UNREGISTER_CVAR("g_DebugTiles");

		UNREGISTER_CVAR("g_FogOfWar");
		UNREGISTER_CVAR("g_FogOfWar_min_x");
		UNREGISTER_CVAR("g_FogOfWar_min_y");
		UNREGISTER_CVAR("g_FogOfWar_max_x");
		UNREGISTER_CVAR("g_FogOfWar_max_y");
	}

};
