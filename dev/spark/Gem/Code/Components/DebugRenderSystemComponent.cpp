
#include "spark_precompiled.h"


#include <ISystem.h>
#include <CryAction.h>
#include <Cry_Camera.h>
#include <IRenderer.h>
#include <MathConversion.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>

#include "DebugRenderSystemComponent.h"
#include "Utils/Log.h"



namespace spark {


	void DebugRenderSystemComponent::Reflect(AZ::ReflectContext* context)
	{
		if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serialize->Class<DebugRenderSystemComponent, AZ::Component>()
				->Version(0)
				;

			if (AZ::EditContext* ec = serialize->GetEditContext())
			{
				ec->Class<DebugRenderSystemComponent>("DebugRenderSystemComponent", "handle timeouts")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					//   ->Attribute(AZ::Edit::Attributes::Category, "spark")
					//	->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
					//;
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true);
			}
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
		{
			behaviorContext->EBus<DebugRenderRequestBus>("DebugRenderRequestBus")
				->Event("Begin", &DebugRenderRequestBus::Events::Begin)
				->Event("DrawSphere", &DebugRenderRequestBus::Events::DrawSphere)
				->Event("DrawLine", &DebugRenderRequestBus::Events::DrawLine)
				->Event("DrawCone", &DebugRenderRequestBus::Events::DrawCone)
				->Event("DrawCylinder", &DebugRenderRequestBus::Events::DrawCylinder)
				->Event("Draw2DText", &DebugRenderRequestBus::Events::Draw2DText)
				->Event("Draw2DRect", &DebugRenderRequestBus::Events::Draw2DRect)
				->Event("DrawDirection", &DebugRenderRequestBus::Events::DrawDirection);

		}
	}

	void DebugRenderSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("DebugRenderSystemService"));
	}

	void DebugRenderSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("DebugRenderSystemService"));
	}

	void DebugRenderSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		(void)required;
	}

	void DebugRenderSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		(void)dependent;
	}

	
	void DebugRenderSystemComponent::Init()
	{
	}

	void DebugRenderSystemComponent::Activate()
	{
		DebugRenderRequestBus::Handler::BusConnect();
	}

	void DebugRenderSystemComponent::Deactivate()
	{
		DebugRenderRequestBus::Handler::BusDisconnect();
	}



	void DebugRenderSystemComponent::Begin(AZStd::string name, bool clear)
	{
		if (auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug())
		{
			pPersistentDebug->Begin(name.c_str(), clear);
		}
	}

	void DebugRenderSystemComponent::DrawSphere(AZ::Vector3 center, float radius,  AZ::Color color, float timeout)
	{
		//sLOG("DebugRenderSystemComponent::DrawSphere called center=" + center + "  radius" + radius);
		if (auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug())
		{
			pPersistentDebug->AddSphere(AZVec3ToLYVec3(center), radius, AZColorToLYColorF(color), timeout);
		}
	}

	void DebugRenderSystemComponent::DrawLine(AZ::Vector3 p1, AZ::Vector3 p2,  AZ::Color color, float timeout)
	{
		if (auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug())
		{
			pPersistentDebug->AddLine(AZVec3ToLYVec3(p1), AZVec3ToLYVec3(p2), AZColorToLYColorF(color), timeout);
		}
	}

	void DebugRenderSystemComponent::DrawCone(AZ::Vector3 pos, AZ::Vector3 dir, float baseRadius, float height, AZ::Color color, float timeout)
	{
		if (auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug())
		{
			pPersistentDebug->AddCone(AZVec3ToLYVec3(pos), AZVec3ToLYVec3(dir),baseRadius,height, AZColorToLYColorF(color), timeout);
		}
	}
	void DebugRenderSystemComponent::DrawCylinder(AZ::Vector3 pos, AZ::Vector3 dir, float radius, float height, AZ::Color color, float timeout)
	{
		if (auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug())
		{
			pPersistentDebug->AddCylinder(AZVec3ToLYVec3(pos), AZVec3ToLYVec3(dir),radius,height, AZColorToLYColorF(color), timeout);
		}
	}
	void DebugRenderSystemComponent::Draw2DText(AZStd::string text, float x, float y, float fontSize, AZ::Color color, float timeout)
	{
		if (auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug())
		{
			pPersistentDebug->AddText(x,y,fontSize, AZColorToLYColorF(color), timeout,text.c_str());
		}
	}
	void DebugRenderSystemComponent::DrawDirection(AZ::Vector3 pos, AZ::Vector3 dir, float radius, AZ::Color color, float timeout)
	{
		if (auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug())
		{
			pPersistentDebug->AddDirection(AZVec3ToLYVec3(pos), radius, AZVec3ToLYVec3(dir), AZColorToLYColorF(color), timeout);
		}
	}

	void DebugRenderSystemComponent::Draw2DRect(float x, float y, float width, float height, AZ::Color color, float timeout)
	{
		if (auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug())
		{
			pPersistentDebug->Add2DRect(x,y,width,height,AZColorToLYColorF(color), timeout);
		}
	}
	

}
