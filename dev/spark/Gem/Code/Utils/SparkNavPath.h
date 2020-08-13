#pragma once

#include <AzCore/std/containers/vector.h>
#include <AzCore/Math/Vector3.h>
#include "PathfinderRequest.h"

namespace spark
{

	using NavigationPathTicket = int;

	class SparkNavPath
	{
	public:
		SparkNavPath() {}

		AZ::Vector3* GetDestinationPoint() 
		{
			return m_pathPoints.empty() ? 0 : &(m_pathPoints[m_pathPoints.size() - 1]);
		}

		const AZ::Vector3* GetNextPathPoint()const {
			return m_pathPoints.size() > 1 ? &(*(m_pathPoints.begin() + 1)) : 0;
		}

		bool Advance(AZ::Vector3& nextPathPoint)
		{
			if (m_pathPoints.size() <= 1)
			{
				//m_valid = false;
				return false;
			}
			m_pathPoints.erase(m_pathPoints.begin());
			if (m_pathPoints.size() == 1)
			{
				m_currentFrac = 0.0f;
				//m_valid = false;
				return false;
			}
			nextPathPoint = *GetNextPathPoint();
			return true;
		}
		bool Advance()
		{
			AZ::Vector3 p;
			return Advance(p);
		}

		void Draw() const
		{
		}
		
		//private:
		AZ::EntityId m_navigationEntity;
		PathfindRequest m_request;
		NavigationPathTicket m_ticket = -1;

		AZStd::vector<AZ::Vector3> m_pathPoints;

		operator bool() const { return m_valid; }
		/// the current position on the path between previous and current point
		float m_currentFrac = 0.0f;
		bool m_valid = false;
		//friend class GridMate::Marshaler<spark::SparkNavPath>;
	};

}


