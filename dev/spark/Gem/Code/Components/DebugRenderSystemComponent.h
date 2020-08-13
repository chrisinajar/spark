#pragma once


#include "Busses/TimerBus.h"
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/containers/list.h>

#include "Busses/DebugRenderBus.h"

namespace spark
{

	class DebugRenderSystemComponent
		: public AZ::Component,
		public DebugRenderRequestBus::Handler
	{
	public:
		AZ_COMPONENT(DebugRenderSystemComponent, "{2EFE6053-88D7-41A0-B118-A61A2F081684}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		void Begin(AZStd::string name, bool clear) override;

		void DrawSphere(AZ::Vector3 center, float radius, AZ::Color color, float timeout) override;
		void DrawLine(AZ::Vector3 p1, AZ::Vector3 p2,  AZ::Color color, float timeout) override;
		void DrawCone(AZ::Vector3 pos, AZ::Vector3 dir, float baseRadius, float height, AZ::Color color, float timeout) override;
		void DrawCylinder(AZ::Vector3 pos, AZ::Vector3 dir, float radius, float height,  AZ::Color color, float timeout) override;
		void Draw2DText(AZStd::string text,float x, float y, float fontSize,  AZ::Color color, float timeout) override;
		void DrawDirection(AZ::Vector3 pos, AZ::Vector3 dir, float radius,  AZ::Color color, float timeout) override;
		void Draw2DRect(float x, float y, float width, float height, AZ::Color color,float timeout) override;
    protected:

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

		
	};


}