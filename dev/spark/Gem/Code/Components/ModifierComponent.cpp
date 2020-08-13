
#include "spark_precompiled.h"

#include "ModifierComponent.h"

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <LmbrCentral/Rendering/ParticleComponentBus.h>

#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <AzFramework/Components/TransformComponent.h>
#include <AzFramework/Entity/EntityContextBus.h>

#include "Busses/GameManagerBus.h"
#include "Busses/UnitBus.h"
#include "Utils/Log.h"
#include "Utils/GridMate.h"
#include "Utils/Marshaler.h"


#include <LmbrCentral/Shape/SphereShapeComponentBus.h>
#include <AzFramework/Components/TransformComponent.h>
#include "TriggerAreaComponent.h"
#include "UnitNavigationComponent.h"


using namespace spark;

void DispelScriptConstructor(Dispel* self, AZ::ScriptDataContext& dc)
{
	*self = Dispel();

	if (dc.GetNumArguments() == 0)
	{
		return;
	}
	else if (dc.GetNumArguments() == 1)
	{
		if (dc.IsNumber(0))
		{
			int level = 0;
			dc.ReadArg(0, level);
			self->level = level;
			return;
		}
	}
	else if (dc.GetNumArguments() == 2)
	{
		if (dc.IsNumber(0) && dc.IsNumber(1))
		{
			int level = 0,flag = 0;
			dc.ReadArg(0, level);
			dc.ReadArg(1, flag);
			self->level = level;
			self->flag = flag;
			return;
		}
	}

	dc.GetScriptContext()->Error(AZ::ScriptContext::ErrorType::Error, true, "Invalid arguments passed to Dispel().");
}

void Dispel::Reflect(AZ::ReflectContext * reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<Dispel>()
			->Version(1)
			->Field("level", &Dispel::level)
			->Field("flag", &Dispel::flag);
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->Class<Dispel>("Dispel")
			->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
			->Attribute(AZ::Script::Attributes::ConstructorOverride, &DispelScriptConstructor)
			->Property("level", BehaviorValueProperty(&Dispel::level))
			->Property("flag", BehaviorValueProperty(&Dispel::flag))
			->Enum<(int)Dispel::DISPEL_FLAG_TOTAL>("DISPEL_FLAG_TOTAL")
			->Enum<(int)Dispel::DISPEL_FLAG_DEATH>("DISPEL_FLAG_DEATH")
			->Enum<(int)Dispel::DISPEL_LEVEL_BASIC>("DISPEL_LEVEL_BASIC")
			->Enum<(int)Dispel::DISPEL_LEVEL_STRONG>("DISPEL_LEVEL_STRONG")
			->Enum<(int)Dispel::DISPEL_LEVEL_MAX>("DISPEL_LEVEL_MAX")

			->Method("DeathDispel", &Dispel::DeathDispel)
			;

	}	
}
AZStd::string Dispel::ToString() const
{
	AZStd::string flagStr;

	if (flag & DISPEL_FLAG_TOTAL) flagStr += "DISPEL_FLAG_TOTAL,";
	if (flag & DISPEL_FLAG_DEATH) flagStr += "DISPEL_FLAG_DEATH,";
	
	if (!flagStr.empty())
	{
		flagStr.pop_back();
		flagStr = AZStd::string::format(",flag:[%s]", flagStr.c_str());
	}
	return AZStd::string::format("Dispel(level=%d%s)", level,flagStr.c_str());
}


/**
*	Interface to lua scripts
*/
class ModifierNotificationBusHandler
	: public ModifierNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(ModifierNotificationBusHandler, "{A043B966-E292-4E42-A97A-C60264BDE800}", AZ::SystemAllocator, OnAttached,OnDetached,OnDestroy,IsDispellable);

	void OnAttached(UnitId id) override
	{
		Call(FN_OnAttached, id);
	}

	void OnDetached(UnitId id) override
	{
		Call(FN_OnDetached, id);
	}
	void OnDestroy() override
	{
		Call(FN_OnDestroy);
	}

	bool IsDispellable(Dispel dispel) override
	{
		bool dispellable = true;
		CallResult(dispellable, FN_IsDispellable, dispel);
		return dispellable;
	}
};


class ModifiersNotificationBusHandler
	: public ModifiersNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(ModifiersNotificationBusHandler, "{E4B1472C-90AC-4EA9-BF27-082B53F76D52}", AZ::SystemAllocator, OnModifierCreated);

	void OnModifierCreated(ModifierId id, ModifierTypeId type)
	{
		Call(FN_OnModifierCreated, id,type);
	}
};

class ModifierTypeNotificationBusHandler
	: public ModifierTypeNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(ModifierTypeNotificationBusHandler, "{CD89C30D-F78F-43A6-93E1-D80DF2FD36AF}", AZ::SystemAllocator, OnModifierCreated);

	void OnModifierCreated(ModifierId id)
	{
		Call(FN_OnModifierCreated, id);
	}
};

class AuraNotificationBusHandler
	: public AuraNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(AuraNotificationBusHandler, "{1F83097D-4C1B-41E7-B09D-6375A8218D79}", AZ::SystemAllocator, IsUnitAffected);

	bool IsUnitAffected(UnitId id) override
	{
		bool affected = true;
		CallResult(affected, FN_IsUnitAffected, id);
		return affected;
	}
};


class ModifierReplicaChunk : public GridMate::ReplicaChunkBase
{
public:
	AZ_CLASS_ALLOCATOR(ModifierReplicaChunk, AZ::SystemAllocator, 0);

	static const char* GetChunkName() { return "ModifierReplicaChunk"; }

	ModifierReplicaChunk()
		: m_parent("Parent")
		, m_ability("Ability")
		, m_caster("Caster")
		, m_visible("Visible")
		, m_iconTexture("IconTexture")
		, m_modifierTypeId("ModifierTypeId")
		, m_auraRange("AuraRange")
		, m_particle("Particle")
	{
	}

	bool IsReplicaMigratable()
	{
		return false;
	}

	GridMate::DataSet<AZ::EntityId>::BindInterface<ModifierComponent, &ModifierComponent::OnNewParent> m_parent;
	GridMate::DataSet<AZ::EntityId>::BindInterface<ModifierComponent, &ModifierComponent::OnNewAbility> m_ability;
	GridMate::DataSet<AZ::EntityId>::BindInterface<ModifierComponent, &ModifierComponent::OnNewCaster> m_caster;
	GridMate::DataSet<bool>::BindInterface<ModifierComponent, &ModifierComponent::OnNewVisible> m_visible;
	GridMate::DataSet<AZStd::string>::BindInterface<ModifierComponent, &ModifierComponent::OnNewIconTexture> m_iconTexture;
	GridMate::DataSet<ModifierTypeId>::BindInterface<ModifierComponent, &ModifierComponent::OnNewModifierTypeId> m_modifierTypeId;
	GridMate::DataSet<float>::BindInterface<ModifierComponent, &ModifierComponent::OnNewAuraRange> m_auraRange;
	GridMate::DataSet<AZStd::string>::BindInterface<ModifierComponent, &ModifierComponent::OnNewParticle> m_particle;
};


void ModifierComponent::Reflect(AZ::ReflectContext* reflection)
{
	Dispel::Reflect(reflection);

	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<ModifierComponent, AzFramework::NetBindable, AZ::Component>()
			->Version(2)
			->Field("parent", &ModifierComponent::m_parent)
			->Field("ability", &ModifierComponent::m_ability)
			->Field("caster", &ModifierComponent::m_caster)
			->Field("visible", &ModifierComponent::m_visible)
			->Field("iconTexture", &ModifierComponent::m_iconTexture)
			->Field("typeId", &ModifierComponent::m_modifierTypeId)
			->Field("auraRange", &ModifierComponent::m_auraRange)
			->Field("particle", &ModifierComponent::m_particle)
			;
		

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<ModifierComponent>("ModifierComponent", "")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				;
		}
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{

		behaviorContext->EBus<ModifierRequestBus>("ModifierRequestBus")
			->Event("SetModifierTypeId", &ModifierRequestBus::Events::SetModifierTypeId)
			->Event("GetModifierTypeId", &ModifierRequestBus::Events::GetModifierTypeId)
			->Event("SetIconTexture", &ModifierRequestBus::Events::SetIconTexture)
			->Event("GetIconTexture", &ModifierRequestBus::Events::GetIconTexture)
			->Event("IsVisible", &ModifierRequestBus::Events::IsVisible)
			->Event("SetVisible", &ModifierRequestBus::Events::SetVisible)
			->Event("SetCaster", &ModifierRequestBus::Events::SetCaster)
			->Event("GetCaster", &ModifierRequestBus::Events::GetCaster)
			->Event("SetParent", &ModifierRequestBus::Events::SetParent)
			->Event("GetParent", &ModifierRequestBus::Events::GetParent)
			->Event("SetAbility", &ModifierRequestBus::Events::SetAbility)
			->Event("GetAbility", &ModifierRequestBus::Events::GetAbility)
			->Event("IsAura", &ModifierRequestBus::Events::IsAura)
			->Event("SetAuraRadius", &ModifierRequestBus::Events::SetAuraRadius)
			->Event("GetAuraRadius", &ModifierRequestBus::Events::GetAuraRadius)
			->Event("SetParticle", &ModifierRequestBus::Events::SetParticle)
			->Event("GetParticle", &ModifierRequestBus::Events::GetParticle)
			->Event("RemoveAura", &ModifierRequestBus::Events::RemoveAura)
			->Event("DoesStack", &ModifierRequestBus::Events::DoesStack)
			->Event("SetDoesStack", &ModifierRequestBus::Events::SetDoesStack)
			->Event("Destroy", &ModifierRequestBus::Events::Destroy)
			->Event("ToString", &ModifierRequestBus::Events::ToString)
			;

		behaviorContext->EBus<ModifierNotificationBus>("ModifierNotificationBus")
			->Handler<ModifierNotificationBusHandler>()
			//->Event("OnAttached", &ModifierNotificationBus::Events::OnAttached)
			//->Event("OnDetached", &ModifierNotificationBus::Events::OnDetached)
			//->Event("OnDestroy", &ModifierNotificationBus::Events::OnDestroy);
			;

		behaviorContext->EBus<ModifiersNotificationBus>("ModifiersNotificationBus")
			->Handler<ModifiersNotificationBusHandler>()
			// ->Event("OnModifierCreated", &ModifiersNotificationBus::Events::OnModifierCreated);
			;

		behaviorContext->EBus<ModifierTypeNotificationBus>("ModifierTypeNotificationBus")
			->Handler<ModifierTypeNotificationBusHandler>()
			//->Event("OnModifierCreated", &ModifierTypeNotificationBus::Events::OnModifierCreated);
			;

		behaviorContext->EBus<AuraNotificationBus>("AuraNotificationBus")
			->Handler<AuraNotificationBusHandler>();
	}

	if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection))
	{
		netContext->Class<ModifierComponent>()
			->Chunk<ModifierReplicaChunk>()
			->Field("Parent", &ModifierReplicaChunk::m_parent)
			->Field("Ability", &ModifierReplicaChunk::m_ability)
			->Field("Caster", &ModifierReplicaChunk::m_caster)
			->Field("Visible", &ModifierReplicaChunk::m_visible)
			->Field("IconTexture", &ModifierReplicaChunk::m_iconTexture)
			->Field("ModifierTypeId", &ModifierReplicaChunk::m_modifierTypeId)
			->Field("AuraRange", &ModifierReplicaChunk::m_auraRange)
			->Field("Particle", &ModifierReplicaChunk::m_particle)
			;
	}
}

void ModifierComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
	provided.push_back(AZ_CRC("ModifierService"));
}
void ModifierComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
	incompatible.push_back(AZ_CRC("ModifierService"));
}
void ModifierComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
	required.push_back(AZ_CRC("VariableHolderService"));
}
void ModifierComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{

}

GridMate::ReplicaChunkPtr ModifierComponent::GetNetworkBinding ()
{
	AZ_Printf(0, "ModifierComponent::GetNetworkBinding");
	auto replicaChunk = GridMate::CreateReplicaChunk<ModifierReplicaChunk>();
	replicaChunk->SetHandler(this);
	m_replicaChunk = replicaChunk;

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		replicaChunk->m_parent.Set(m_parent);
		replicaChunk->m_ability.Set(m_ability);
		replicaChunk->m_caster.Set(m_caster);
		replicaChunk->m_visible.Set(m_visible);
		replicaChunk->m_iconTexture.Set(m_iconTexture);
		replicaChunk->m_modifierTypeId.Set(m_modifierTypeId);
		replicaChunk->m_auraRange.Set(m_auraRange);
		replicaChunk->m_particle.Set(m_particle);
	}
	else if (m_active)
	{
		SetAbility(replicaChunk->m_ability.Get());
		SetCaster(replicaChunk->m_caster.Get());
		SetVisible(replicaChunk->m_visible.Get());
		SetIconTexture(replicaChunk->m_iconTexture.Get());
		SetModifierTypeId(replicaChunk->m_modifierTypeId.Get());
		SetAuraRadius(replicaChunk->m_auraRange.Get());
		SetParent(replicaChunk->m_parent.Get());
		SetParticle(replicaChunk->m_particle.Get());
	}

	return m_replicaChunk;
}
void ModifierComponent::SetNetworkBinding (GridMate::ReplicaChunkPtr chunk)
{
	AZ_Printf(0, "ModifierComponent::SetNetworkBinding");
	chunk->SetHandler(this);
	m_replicaChunk = chunk;

	ModifierReplicaChunk* replicaChunk = static_cast<ModifierReplicaChunk*>(m_replicaChunk.get());

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		replicaChunk->m_parent.Set(m_parent);
		replicaChunk->m_ability.Set(m_ability);
		replicaChunk->m_caster.Set(m_caster);
		replicaChunk->m_visible.Set(m_visible);
		replicaChunk->m_iconTexture.Set(m_iconTexture);
		replicaChunk->m_modifierTypeId.Set(m_modifierTypeId);
		replicaChunk->m_auraRange.Set(m_auraRange);
		replicaChunk->m_particle.Set(m_particle);
	}
	else if (m_active)
	{
		SetAbility(replicaChunk->m_ability.Get());
		SetCaster(replicaChunk->m_caster.Get());
		SetVisible(replicaChunk->m_visible.Get());
		SetIconTexture(replicaChunk->m_iconTexture.Get());
		SetModifierTypeId(replicaChunk->m_modifierTypeId.Get());
		SetAuraRadius(replicaChunk->m_auraRange.Get());
		SetParent(replicaChunk->m_parent.Get());
		SetParticle(replicaChunk->m_particle.Get());
	}
}
void ModifierComponent::UnbindFromNetwork ()
{
	AZ_Printf(0, "ModifierComponent::UnbindFromNetwork");
	m_replicaChunk->SetHandler(nullptr);
	m_replicaChunk = nullptr;
}
void ModifierComponent::OnNewParent (const AZ::EntityId &parent, const GridMate::TimeContext&)
{
	AZ_Printf(0, "Got new Parent!");
	SetParent(parent);
}
void ModifierComponent::OnNewAbility (const AZ::EntityId &ability, const GridMate::TimeContext&)
{
	AZ_Printf(0, "Got new Ability!");
	SetAbility(ability);
}
void ModifierComponent::OnNewCaster (const AZ::EntityId &caster, const GridMate::TimeContext&)
{
	AZ_Printf(0, "Got new Caster!");
	SetCaster(caster);
}
void ModifierComponent::OnNewVisible (const bool &visible, const GridMate::TimeContext&)
{
	AZ_Printf(0, "Got new Visible!");
	SetVisible(visible);
}
void ModifierComponent::OnNewIconTexture (const AZStd::string &iconTexture, const GridMate::TimeContext&)
{
	AZ_Printf(0, "Got new IconTexture!");
	SetIconTexture(iconTexture);
}
void ModifierComponent::OnNewModifierTypeId (const ModifierTypeId &modifierTypeId, const GridMate::TimeContext&)
{
	AZ_Printf(0, "Got new ModifierTypeId!");
	SetModifierTypeId(modifierTypeId);
}
void ModifierComponent::OnNewAuraRange (const float &auraRange, const GridMate::TimeContext&)
{
	AZ_Printf(0, "Got new AuraRange!");
	SetAuraRadius(auraRange);
}
void ModifierComponent::OnNewParticle (const AZStd::string &particle, const GridMate::TimeContext&)
{
	AZ_Printf(0, "Got new AuraRange!");
	SetParticle(particle);
}

void ModifierComponent::Init()
{
	//sLOG("ModifierComponent::Init");
}
void ModifierComponent::Activate()
{
	//sLOG("ModifierComponent::Activate");
	ModifierRequestBus::Handler::BusConnect(GetEntityId());
	CooldownNotificationBus::Handler::BusConnect(GetEntityId());

	m_active = true;

	if (m_replicaChunk && !AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		ModifierReplicaChunk* replicaChunk = static_cast<ModifierReplicaChunk*>(m_replicaChunk.get());
		SetAbility(replicaChunk->m_ability.Get());
		SetCaster(replicaChunk->m_caster.Get());
		SetVisible(replicaChunk->m_visible.Get());
		SetIconTexture(replicaChunk->m_iconTexture.Get());
		SetModifierTypeId(replicaChunk->m_modifierTypeId.Get());
		SetAuraRadius(replicaChunk->m_auraRange.Get());
		SetParent(replicaChunk->m_parent.Get());
	}
}
void ModifierComponent::Deactivate()
{
	if (!m_ignoreDeactivate)
	{
		sLOG("deactivating \"" + m_modifierTypeId + "\" " + GetEntityId());

		RemoveAura();

		if (m_parent.IsValid())
		{
			EBUS_EVENT_ID(m_parent, UnitRequestBus, RemoveModifier, GetEntityId());
		}
		EBUS_EVENT_ID(GetEntityId(), ModifierNotificationBus, OnDestroy);
	}

	m_active = false;
	//sLOG("ModifierComponent::Deactivate");
	CooldownNotificationBus::Handler::BusDisconnect();
	ModifierRequestBus::Handler::BusDisconnect();
}

void ModifierComponent::SetModifierTypeId(ModifierTypeId modifierTypeId)
{
	SPARK_UPDATE_VALUE(Modifier, modifierTypeId);
}

ModifierTypeId ModifierComponent::GetModifierTypeId()
{
	return m_modifierTypeId;
}

void ModifierComponent::SetIconTexture(AZStd::string iconTexture)
{
	SPARK_UPDATE_VALUE(Modifier, iconTexture);
	m_iconTexture = iconTexture;
}
AZStd::string ModifierComponent::GetIconTexture()
{
	return m_iconTexture;
}

bool ModifierComponent::IsVisible()
{
	return m_visible;
}
void ModifierComponent::SetVisible(bool visible)
{
	SPARK_UPDATE_VALUE(Modifier, visible);
}

bool ModifierComponent::DoesStack()
{
	return m_doesStack;
}

void ModifierComponent::SetDoesStack(bool stack)
{
	m_doesStack = stack;
}

void ModifierComponent::SetCaster(UnitId caster)
{
	SPARK_UPDATE_VALUE(Modifier, caster);
}
UnitId ModifierComponent::GetCaster()
{
	return m_caster;
}

void ModifierComponent::SetParent(AZ::EntityId parent)
{
	if (m_parent == parent) return;

	//sLOG("ModifierComponent::SetParent");
	if (m_parent.IsValid())
	{
		EBUS_EVENT_ID(GetEntityId(), ModifierNotificationBus, OnDetached, m_parent);
		EBUS_EVENT_ID(m_parent, UnitNotificationBus, OnModifierDetached, GetEntityId());
	}
	SPARK_UPDATE_VALUE(Modifier, parent);

	if (parent.IsValid())
	{
		EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetParentRelative, m_parent);
		EBUS_EVENT_ID(GetEntityId(), ModifierNotificationBus, OnAttached, m_parent);
		//sLOG("ModifierComponent::SetParent sending OnModifierAttached");
		EBUS_EVENT_ID(m_parent, UnitNotificationBus, OnModifierAttached, GetEntityId());
	}
}
AZ::EntityId ModifierComponent::GetParent()
{
	return m_parent;
}

void ModifierComponent::SetAbility(AbilityId ability)
{
	SPARK_UPDATE_VALUE(Modifier, ability);
}
AbilityId ModifierComponent::GetAbility()
{
	return m_ability;
}

//aura
bool ModifierComponent::IsAura()
{
	return m_auraRange > 0.0f;
}
void ModifierComponent::SetAuraRadius(float auraRange)
{
	auraRange = AZStd::max(0.0f, auraRange);
	if (fabs(m_auraRange - auraRange) < 0.0001f)
	{
		return;
	}
	SPARK_UPDATE_VALUE(Modifier, auraRange);

	// everything after this is authoritative only...
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}

	if (IsAura())
	{


		
		if (!m_hasAuraComponents)
		{
			sLOG(" ModifierComponent::SetAuraRadius adding missing components");

			m_ignoreDeactivate = true;

			//check if we got the components we need
			AZ::Entity *e = GetEntity();
			
			auto *transformComponent = e->FindComponent(AZ::TransformComponentTypeId);
			auto *sphereShapeComponent = e->FindComponent(LmbrCentral::SphereShapeComponentTypeId);
			auto *navEntityComponent = e->FindComponent<NavigationEntityComponent>();
			auto *triggerAreaComponent = e->FindComponent<Spark2dTriggerAreaComponent>();

			//add the missing component/s
			e->Deactivate();
			if (!transformComponent)transformComponent = e->CreateComponent(AZ::TransformComponentTypeId);
			if (!sphereShapeComponent)sphereShapeComponent = e->CreateComponent(LmbrCentral::SphereShapeComponentTypeId);
			if (!navEntityComponent) navEntityComponent = e->CreateComponent<NavigationEntityComponent>();
			if (!triggerAreaComponent)triggerAreaComponent = e->CreateComponent<Spark2dTriggerAreaComponent>();
			

			AZ_Assert(transformComponent,   "ModifierComponent::SetAuraRadius   invalid TransformComponent");	
			AZ_Assert(sphereShapeComponent, "ModifierComponent::SetAuraRadius   invalid SphereShapeComponent");
			AZ_Assert(navEntityComponent, "ModifierComponent::SetAuraRadius   invalid NavigationEntityComponent");
			AZ_Assert(triggerAreaComponent, "ModifierComponent::SetAuraRadius   invalid Spark2dTriggerAreaComponent");

			navEntityComponent->m_addOnActivate = false;

			e->InvalidateDependencies();
			e->Activate();

			EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetParentRelative, m_parent);

			//having a parent influence also the scale, changing the radius of the aura. Here we prevent that
			AZ::Transform transform;
			EBUS_EVENT_ID_RESULT(transform, GetEntityId(), AZ::TransformBus, GetWorldTM);
			transform.ExtractScaleExact(); //this function returns the scale and set it to zero
			EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetWorldTM, transform);

			EBUS_EVENT_ID(GetEntityId(), LmbrCentral::TriggerAreaRequestsBus, AddRequiredTag, LmbrCentral::Tag("unit"));

			m_hasAuraComponents = true;
			m_ignoreDeactivate = false;
		}

		if (!LmbrCentral::TriggerAreaNotificationBus::Handler::BusIsConnected())
		{
			sLOG("modifier connected to TriggerAreaNotificationBus");
			LmbrCentral::TriggerAreaNotificationBus::Handler::BusConnect(GetEntityId());
		}

		EBUS_EVENT_ID(GetEntityId(), LmbrCentral::SphereShapeComponentRequestsBus, SetRadius, m_auraRange);
		EBUS_EVENT_ID(GetEntityId(), NavigationEntityRequestBus, AddToNavigationManager);
	}
	else
	{
		RemoveAura();
	}
}

float ModifierComponent::GetAuraRadius()
{
	return m_auraRange;
}

void ModifierComponent::RemoveAura()
{
	float auraRange = 0.f;
	SPARK_UPDATE_VALUE(Modifier, auraRange);

	EBUS_EVENT_ID(GetEntityId(), NavigationEntityRequestBus, RemoveFromNavigationManager);
	LmbrCentral::TriggerAreaNotificationBus::Handler::BusDisconnect();

	//if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	//{
	//	for(auto it:m_auraModifiers)
	//	{
	//		OnTriggerAreaExited(it.first);
	//		/*ModifierNotificationBus::MultiHandler::BusDisconnect(it.second);
	//		EBUS_EVENT_ID(it.first, UnitRequestBus, RemoveModifier, it.second);
	//		EBUS_EVENT_ID(it.second, ModifierRequestBus, Destroy);*/
	//	}
	//	m_auraModifiers.clear();
	//}
}


void ModifierComponent::SetParticle(AZStd::string particle)
{
	SPARK_UPDATE_VALUE(Modifier, particle);
	// macro returns when value hasn't changed
	// update particle effect on local unit

	if (!m_needsParticleEffect)
	{
		AZ::TickBus::Handler::BusConnect();
	}

	// do this async because this function can run before the components are actually initialized
	// tick handler only runs on fully initialized stuff
	m_needsParticleEffect = true;
}

void ModifierComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	if (m_needsParticleEffect)
	{
		m_needsParticleEffect = false;
		AZ::TickBus::Handler::BusDisconnect();

		if (!m_particle.empty())
		{
    		AZ::Vector3 currentPosition;
    		AZ::Vector3 parentPosition;
			// particle effect
    		EBUS_EVENT_ID_RESULT(currentPosition, GetEntityId(), AZ::TransformBus, GetWorldTranslation);

    		EBUS_EVENT_ID_RESULT(parentPosition, m_caster, AZ::TransformBus, GetWorldTranslation);
			AZ_Printf(0, "Adding the particle effect %s %f,%f vs %f,%f", m_particle.c_str(), (float)currentPosition.GetX(), (float)currentPosition.GetY(), (float)parentPosition.GetX(), (float)parentPosition.GetY());

    		EBUS_EVENT_ID_RESULT(parentPosition, m_parent, AZ::TransformBus, GetWorldTranslation);
			AZ_Printf(0, "Adding the particle effect %s %f,%f vs %f,%f", m_particle.c_str(), (float)currentPosition.GetX(), (float)currentPosition.GetY(), (float)parentPosition.GetX(), (float)parentPosition.GetY());

			LmbrCentral::ParticleEmitterSettings particleSettings;
			particleSettings.m_selectedEmitter = m_particle;
			particleSettings.m_sizeScale = 1.0f;
			EBUS_EVENT_ID(GetEntityId(), LmbrCentral::ParticleComponentRequestBus, SetupEmitter, particleSettings.m_selectedEmitter, particleSettings);
			EBUS_EVENT_ID(GetEntityId(), LmbrCentral::ParticleComponentRequestBus, Show);
		}
		else
		{
			EBUS_EVENT_ID(GetEntityId(), LmbrCentral::ParticleComponentRequestBus, Hide);
		}
	}
}

AZStd::string ModifierComponent::GetParticle()
{
	return m_particle;
}

AZStd::string ModifierComponent::ToString()
{
	return AZStd::string::format("Modifier(type=\"%s\",visible=%s)",m_modifierTypeId.c_str(),m_visible?"true":"false");
}

void ModifierComponent::IncrementReferenceCounter()
{
	++m_referenceCounter;
	SetDuration(0);
}

void ModifierComponent::DecrementReferenceCounter()
{
	--m_referenceCounter;
}

int ModifierComponent::GetReferenceCounter()
{
	return m_referenceCounter;
}

void ModifierComponent::SetDuration(float duration)
{
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "cooldown_current"), duration);
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "cooldown_timer"),   duration);
}

void ModifierComponent::OnCooldownFinished()
{
	//DecrementCounter();
	Destroy();
}

void ModifierComponent::OnTriggerAreaEntered(AZ::EntityId enteringEntityId)
{
	sLOG(" ModifierComponent::OnTriggerAreaEntered " + enteringEntityId);

	//ignore the unit having the aura
	if (enteringEntityId == m_parent)return;

	////Check if it's a unit
	//bool ok=false;
	//EBUS_EVENT_ID_RESULT(ok, enteringEntityId, LmbrCentral::TagComponentRequestBus, HasTag, LmbrCentral::Tag("unit"));
	//if (!ok)return;

	//check if the unit already has the aura applied
	auto it = m_auraModifiers.find(enteringEntityId);
	if (it != m_auraModifiers.end()) return;

	bool affected = true;
	EBUS_EVENT_ID_RESULT(affected, GetEntityId(), AuraNotificationBus, IsUnitAffected, enteringEntityId);
	if (!affected)return;


	ModifierId modifierId;

	if (!DoesStack())
	{
		EBUS_EVENT_ID_RESULT(modifierId, enteringEntityId, UnitRequestBus, FindModifierByTypeId, GetModifierTypeId());

		if (modifierId.IsValid())
		{
			sLOG("ModifierComponent::OnTriggerAreaEntered entity already has the (non stacking) modifier, I'm just incrementing the counter");
			EBUS_EVENT_ID(modifierId, ModifierRequestBus, IncrementReferenceCounter);
		}
	}
	
	if (!modifierId.IsValid())
	{	
		EBUS_EVENT_RESULT(modifierId, GameManagerRequestBus, CreateModifier, GetCaster(), GetAbility(), GetModifierTypeId());

		EBUS_EVENT_ID(enteringEntityId, UnitRequestBus, AddModifier, modifierId);

		sLOG(" ModifierComponent::OnTriggerAreaEntered applying the aura modifier(" + modifierId + ")  to unit:" + enteringEntityId);
	}

	m_auraModifiers[enteringEntityId] = modifierId;
	ModifierNotificationBus::MultiHandler::BusConnect(modifierId);
}

void ModifierComponent::OnTriggerAreaExited(AZ::EntityId exitingEntityId)
{
	sLOG(" ModifierComponent::OnTriggerAreaExited "+ exitingEntityId);

	////Check if it's a unit
	//bool ok=false;
	//EBUS_EVENT_ID_RESULT(ok, exitingEntityId, LmbrCentral::TagComponentRequestBus, HasTag, LmbrCentral::Tag("unit"));
	//if (!ok)return;

	auto it = m_auraModifiers.find(exitingEntityId);
	if (it != m_auraModifiers.end())
	{
		ModifierId modifierId = it->second;
		ModifierNotificationBus::MultiHandler::BusDisconnect(it->second);
		m_auraModifiers.erase(it);


		EBUS_EVENT_ID(modifierId, ModifierRequestBus, DecrementReferenceCounter);

		int counter = 0;
		EBUS_EVENT_ID_RESULT(counter, modifierId, ModifierRequestBus, GetReferenceCounter);
		if (counter<=0)
		{
			DestroyAfterLingerDuration(modifierId);
		}
	}
}

void ModifierComponent::OnDetached(UnitId unitId)
{
	ModifierId id = *ModifierNotificationBus::GetCurrentBusId();
	
	auto it = m_auraModifiers.find(unitId);
	if (it != m_auraModifiers.end())
	{
		AZ_Warning("ModifierComponent", it->second == id, " OnDetached called but modifierId(address) and the one stored do not match");
		m_auraModifiers.erase(it);
		ModifierNotificationBus::MultiHandler::BusDisconnect(it->second);

		AZStd::vector<AZ::EntityId> entities;
		EBUS_EVENT_ID_RESULT(entities, GetEntityId(), LmbrCentral::TriggerAreaRequestsBus, GetEntitiesInside);
		auto inside = AZStd::find(entities.begin(), entities.end(), unitId);
		if (inside != entities.end())
		{
			OnTriggerAreaEntered(unitId);
		}
	}
}

void ModifierComponent::DestroyAfterLingerDuration(ModifierId modifierId)
{
	float linger_duration = 0.5;
	bool defined = false;

	EBUS_EVENT_RESULT(defined, VariableManagerRequestBus, VariableExists, VariableId(GetEntityId(), "linger_duration"));
	if (defined)
	{
		EBUS_EVENT_RESULT(linger_duration, VariableManagerRequestBus, GetValue, VariableId(GetEntityId(), "linger_duration"));
	}

	if (linger_duration < 0.0f)
	{
		return;
	}
	else if (linger_duration == 0.0f)
	{
		EBUS_EVENT_ID(modifierId, ModifierRequestBus, Destroy);
	}
	else //if (linger_duration > 0.0f)
	{
		EBUS_EVENT_ID(modifierId, ModifierRequestBus, SetDuration, linger_duration);
	}
	
}

void ModifierComponent::Destroy()
{
	sLOG("destroying \""+m_modifierTypeId + "\" "+GetEntityId());
	EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntityAndDescendants, GetEntityId());
}
