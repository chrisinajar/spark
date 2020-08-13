#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Vector4.h>

namespace spark {

	class DebugRenderRequests
		: public AZ::EBusTraits
	{
	public:
		// One handler is supported.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		

		virtual void Begin(AZStd::string name, bool clear) = 0;

		virtual void DrawSphere(AZ::Vector3 center, float radius,  AZ::Color color, float timer) = 0;
		virtual void DrawLine(AZ::Vector3 p1, AZ::Vector3 p2,  AZ::Color color, float timer) = 0;
		virtual void DrawCone(AZ::Vector3 pos, AZ::Vector3 dir, float baseRadius, float height, AZ::Color color, float timer) = 0;
		virtual void DrawCylinder(AZ::Vector3 pos, AZ::Vector3 dir, float radius, float height,  AZ::Color color, float timer) = 0;
		virtual void Draw2DText(AZStd::string text,float x, float y, float fontSize,  AZ::Color color, float timer) = 0;
		virtual void DrawDirection(AZ::Vector3 pos, AZ::Vector3 dir, float radius,  AZ::Color color, float timer) = 0;
		virtual void Draw2DRect(float x, float y, float width, float height, AZ::Color color, float timeout) = 0;
		
	};
	using DebugRenderRequestBus = AZ::EBus<DebugRenderRequests>;


	

}