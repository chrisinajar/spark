
#include "PhysicsWrapper_precompiled.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <physinterface.h>
#include <IPhysics.h>
#include <MathConversion.h>
#include <CryAction.h>
#include <Cry_Camera.h>
#include <ISystem.h>
#include "PhysicsWrapperSystemComponent.h"

namespace PhysicsWrapper
{
    void PhysicsWrapperSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<PhysicsWrapperSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<PhysicsWrapperSystemComponent>("PhysicsWrapper", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void PhysicsWrapperSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("PhysicsWrapperService"));
    }

    void PhysicsWrapperSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("PhysicsWrapperService"));
    }

    void PhysicsWrapperSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        (void)required;
    }

    void PhysicsWrapperSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void PhysicsWrapperSystemComponent::Init()
    {
    }

    void PhysicsWrapperSystemComponent::Activate()
    {
        PhysicsWrapperRequestBus::Handler::BusConnect();
    }

    void PhysicsWrapperSystemComponent::Deactivate()
    {
        PhysicsWrapperRequestBus::Handler::BusDisconnect();
    }




	Hit PhysicsWrapperSystemComponent::PerformRayCastWithParam(float distance, int collisionFlags)
	{
		auto hit = AZStd::make_unique<ray_hit>();

		auto &cam = GetISystem()->GetViewCamera();

		auto direction = cam.GetViewdir();
		auto start = cam.GetPosition() + direction;

		auto pWorld = gEnv->pPhysicalWorld;
		auto numHits = pWorld->RayWorldIntersection(start, direction * distance, ent_all, collisionFlags, hit.get(), 1);

		Hit phit;

		if (numHits > 0)
		{
			phit.position = LYVec3ToAZVec3(hit->pt);
			phit.normal = LYVec3ToAZVec3(hit->n);

			if (hit->pCollider)
			{
				phit.hit = true;
				phit.entity = static_cast<AZ::EntityId>(hit->pCollider->GetForeignData(PHYS_FOREIGN_ID_COMPONENT_ENTITY));
			}
		}

#if !defined(_RELEASE)
		if (auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug())
		{
			const ColorF green(0.000f, 1.000f, 0.000f);
			const ColorF red(1.000f, 0.000f, 0.000f);

			pPersistentDebug->Begin("FG_Line", true);

			Vec3 end = hit->pt;


			if (!!phit.hit && phit.entity.IsValid())
			{
				pPersistentDebug->AddLine(start, end, green, 500);
			}
			else
			{
				pPersistentDebug->AddLine(start, end, red, 500);
			}
		}
#endif

		return phit;
	}

	Hit PhysicsWrapperSystemComponent::PerformRayCast()
	{
		return PerformRayCastWithParam(100.f, rwi_stop_at_pierceable | rwi_colltype_any | rwi_ignore_back_faces);// rwi_stop_at_pierceable | rwi_colltype_any);
	}

	Hit PhysicsWrapperSystemComponent::PerformSingleResultRayCast(AZ::Vector3 start, AZ::Vector3 end)
	{

		const unsigned int collisionFlags = rwi_stop_at_pierceable | rwi_colltype_any | rwi_ignore_back_faces;
		const int entityTypes = ent_all;

		auto direction = end - start;
		
		auto hit = AZStd::make_unique<ray_hit>();
		auto pWorld = gEnv->pPhysicalWorld;
		auto numHits = pWorld->RayWorldIntersection(AZVec3ToLYVec3(start), AZVec3ToLYVec3(direction), entityTypes, collisionFlags, hit.get(), 1);

		Hit phit;

		if (numHits > 0)
		{
			phit.position = LYVec3ToAZVec3(hit->pt);
			phit.normal = LYVec3ToAZVec3(hit->n);

			if (hit->pCollider)
			{
				phit.hit = true;
				phit.entity = static_cast<AZ::EntityId>(hit->pCollider->GetForeignData(PHYS_FOREIGN_ID_COMPONENT_ENTITY));
			}
		}

#if !defined(_RELEASE)
		if (auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug())
		{
			const ColorF green(0.000f, 1.000f, 0.000f);
			const ColorF red(1.000f, 0.000f, 0.000f);

			pPersistentDebug->Begin("FG_Line1", true);

			//Vec3 end = hit->pt;
			

			if (!!phit.hit && phit.entity.IsValid())
			{
				pPersistentDebug->AddLine(hit->pt, hit->pt + hit->n, green, 500);
			}
		}
#endif

		return phit;
	}
}
