#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace spark
{
	class CameraRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual void SetFocus(bool) = 0;
		virtual bool HasFocus() = 0;

		virtual void SetCameraMovementSpeed(float speed) = 0;
		virtual float GetCameraMovevementSpeed() = 0;

		virtual AZ::Vector3 ConvertPixelToWorldCoordinates(AZ::Vector3) = 0;
		virtual AZ::Vector3 ConvertWorldToPixelCoordinates(AZ::Vector3) = 0;

		virtual AZ::Vector3 TerrainRayCast(AZ::Vector2) = 0; //return the position where the raycast hit the terrain

		virtual AZ::Vector2 GetScreenSize() = 0;
		virtual bool IsPointVisible(AZ::Vector3) = 0;
		
		virtual void LookAtWorldPosition(AZ::Vector3) = 0;

		virtual AZ::EntityId GetCameraEntityId() = 0;
	};
	using CameraRequestBus = AZ::EBus<CameraRequests>;
}
