#pragma once

namespace spark
{
	class SparkCVars
	{
	public:
		static const char* PLAYER_COUNT_CVAR;
		
		SparkCVars();
		~SparkCVars();
		
		void RegisterCVars();
		void UnregisterCVars();
	};
};
