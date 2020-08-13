#pragma once
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/bitset.h>

namespace spark
{
	/**
	* Represents a request as submitted by a user of this component, can be used to configure the pathfinding
	* queries by providing overrides for some default values as entered in the editor.
	* Extension of the class in Lmbr
	*/
	class PathfindRequest
	{
	public:

		using NavigationRequestId = AZ::u32;

		///////////////////////////////////////////////////////////////////////////
		// Destination

		void SetDestinationLocation(const AZ::Vector3& destination)
		{
			if (!m_requestMask[RequestBits::DestinationEntity])
			{
				m_destination = destination;
				m_requestMask.set(RequestBits::DestinationPosition);
			}
		}

		AZ::Vector3 GetDestinationLocation() const
		{
			// If there is a target entity or a destination position
			if (m_requestMask[RequestBits::DestinationEntity]
				|| m_requestMask[RequestBits::DestinationPosition])
			{
				return m_destination;
			}
			else
			{
				return AZ::Vector3::CreateZero();
			}
		}

		void SetTargetEntityId(AZ::EntityId targetEntity)
		{
			if (m_requestMask[RequestBits::DestinationPosition])
			{
				m_requestMask.reset(RequestBits::DestinationPosition);
			}

			m_targetEntityId = targetEntity;
			m_requestMask.set(RequestBits::DestinationEntity);
		}

		AZ::EntityId GetTargetEntityId() const
		{
			if (m_requestMask[RequestBits::DestinationEntity])
			{
				return m_targetEntityId;
			}
			else
			{
				return AZ::EntityId();
			}
		}

		///////////////////////////////////////////////////////////////////////////

		void SetArrivalDistanceThreshold(float arrivalDistanceThreshold)
		{
			m_arrivalDistanceThreshold = arrivalDistanceThreshold;
			m_requestMask.set(RequestBits::ArrivalDistanceThreshold);
		}

		float GetArrivalDistanceThreshold() const
		{
			return m_arrivalDistanceThreshold;
		}

		PathfindRequest()
			: m_arrivalDistanceThreshold(0)
			, m_destination(AZ::Vector3::CreateZero())
		{
		}

		bool HasTargetEntity() const
		{
			return m_requestMask[RequestBits::DestinationEntity];
		}

		bool HasTargetLocation() const
		{
			return m_requestMask[RequestBits::DestinationPosition];
		}

		bool HasOverrideArrivalDistance() const
		{
			return m_requestMask[RequestBits::ArrivalDistanceThreshold];
		}


		bool HasOverrideDestinationDirection() const
		{
			return m_requestMask[RequestBits::DestinationDirection];
		}

		void SetDestinationDirection(AZ::Vector3 direction)
		{
			m_destinationDirection = direction;
			m_requestMask.set(RequestBits::DestinationDirection);
		}

		AZ::Vector3 GetDestinationDirection() const
		{
			return m_destinationDirection;
		}


		bool IsValid() const
		{
			return HasTargetLocation() || (HasTargetEntity() && GetTargetEntityId().IsValid());
		}

	private:

		enum RequestBits
		{
			DestinationPosition,
			DestinationEntity,
			ArrivalDistanceThreshold,
			DestinationDirection
		};
		/** Mask used to identify the values that were set / not set in any given request
		* bit 0 : Vector 3 of the destination location
		* bit 1 : AZ::Entity that is to be followed
		* Note: Bit 0 and 1 are mutually exclusive , if both are set bit 1 takes precedence
				So if both a Destination Entity and Location are set, only the Destination Entity is used
		* bit 2 : The override Arrival distance threshold
		*/
		AZStd::bitset<8> m_requestMask;

		//! Destination as set by the requester
		AZ::Vector3 m_destination;

		//! Destination entity
		AZ::EntityId m_targetEntityId;

		// Request configuration parameters
		//! Distance from target at which path traversing is considered complete
		float m_arrivalDistanceThreshold;

		AZ::Vector3 m_destinationDirection;
	};
}
