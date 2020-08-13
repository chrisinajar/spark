
#include "spark_precompiled.h"

#include "TriggerAreaComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Component/TickBus.h>

#include <MathConversion.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzFramework/Network/NetBindingSystemBus.h>
#include <AzFramework/Network/NetworkContext.h>

#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/RemoteProcedureCall.h>
#include <GridMate/Serialize/MarshalerTypes.h>
#include <GridMate/Serialize/MathMarshal.h>
#include <GridMate/Serialize/DataMarshal.h>
#include <GridMate/Serialize/ContainerMarshal.h>

#include <LmbrCentral/Shape/ShapeComponentBus.h>
#include <LmbrCentral/Shape/BoxShapeComponentBus.h>
#include <LmbrCentral/Shape/SphereShapeComponentBus.h>
#include <LmbrCentral/Shape/CapsuleShapeComponentBus.h>
#include <LmbrCentral/Shape/CylinderShapeComponentBus.h>
#include <LmbrCentral/Shape/CompoundShapeComponentBus.h>

#include "TriggerAreaComponent.h"
#include "Utils/NavigationUtils.h"

#include <IRenderAuxGeom.h>
#include <IActorSystem.h>


using namespace LmbrCentral;

namespace spark
{

	//=========================================================================
	// ReplicaChunk
	//=========================================================================
	class TriggerAreaReplicaChunk : public GridMate::ReplicaChunkBase
	{
	public:
		AZ_CLASS_ALLOCATOR(TriggerAreaReplicaChunk, AZ::SystemAllocator, 0);

		static const char* GetChunkName() { return "Spark2dTriggerAreaChunk"; }

		TriggerAreaReplicaChunk()
			: OnAreaEnter("OnAreaEnter")
			, OnAreaExit("OnAreaExit")
		{
		}

		bool IsReplicaMigratable() override
		{
			return true;
		}

		void OnReplicaChangeOwnership(const GridMate::ReplicaContext& /*rc*/) override
		{

		}

		GridMate::Rpc< GridMate::RpcArg<AZ::u64> >::BindInterface<Spark2dTriggerAreaComponent, &Spark2dTriggerAreaComponent::OnEntityEnterAreaRPC> OnAreaEnter;
		GridMate::Rpc< GridMate::RpcArg<AZ::u64> >::BindInterface<Spark2dTriggerAreaComponent, &Spark2dTriggerAreaComponent::OnEntityExitAreaRPC> OnAreaExit;

		// Only used to pass along the initial set, after that all of the enter/exits will take care of updating the list.
		AZStd::vector< AZ::u64 > m_initialEntitiesInArea;
	};

	class TriggerAreaReplicaChunkDesc : public AzFramework::ExternalChunkDescriptor<TriggerAreaReplicaChunk>
	{
	public:

		GridMate::ReplicaChunkBase* CreateFromStream(GridMate::UnmarshalContext& context) override
		{
			TriggerAreaReplicaChunk* triggerAreaChunk = aznew TriggerAreaReplicaChunk;

			AZStd::vector<AZ::u64> initialEntitiesInArea;

			context.m_iBuf->Read(initialEntitiesInArea);

			triggerAreaChunk->m_initialEntitiesInArea.reserve(initialEntitiesInArea.size());

			for (AZ::u64 entityId : initialEntitiesInArea)
			{
				triggerAreaChunk->m_initialEntitiesInArea.push_back(entityId);
			}

			return triggerAreaChunk;
		}

		void DiscardCtorStream(GridMate::UnmarshalContext& context) override
		{
			AZStd::vector< AZ::u64 > discard;
			context.m_iBuf->Read(discard);
		}

		void MarshalCtorData(GridMate::ReplicaChunkBase* chunk, GridMate::WriteBuffer& wb) override
		{
			TriggerAreaReplicaChunk* triggerAreaChunk = static_cast<TriggerAreaReplicaChunk*>(chunk);
			Spark2dTriggerAreaComponent* spark2dTriggerAreaComponent = static_cast<Spark2dTriggerAreaComponent*>(chunk->GetHandler());

			if (spark2dTriggerAreaComponent)
			{
				AZStd::vector< AZ::u64 > entitiesInside;
				entitiesInside.reserve(spark2dTriggerAreaComponent->m_entitiesInside.size());

				for (AZ::EntityId entityId : spark2dTriggerAreaComponent->m_entitiesInside)
				{
					AZ::u64 writableId = static_cast<AZ::u64>(entityId);
					entitiesInside.push_back(writableId);
				}

				wb.Write(entitiesInside);
			}
			else
			{
				wb.Write(triggerAreaChunk->m_initialEntitiesInArea);
			}
		}

		void DeleteReplicaChunk(GridMate::ReplicaChunkBase* replica)
		{
			delete replica;
		}
	};



	//=========================================================================
	// Reflect
	//=========================================================================
	void Spark2dTriggerAreaComponent::Reflect(AZ::ReflectContext* context)
	{
		
		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serializationContext->Class<Spark2dTriggerAreaComponent, AZ::Component, AzFramework::NetBindable>()
				->Version(2)
				->Field("TriggerOnce", &Spark2dTriggerAreaComponent::m_triggerOnce)
				->Field("ActivatedBy", &Spark2dTriggerAreaComponent::m_activationEntityType)
				->Field("SpecificInteractEntities", &Spark2dTriggerAreaComponent::m_specificInteractEntities)
				->Field("RequiredTags", &Spark2dTriggerAreaComponent::m_requiredTags)
				->Field("ExcludedTags", &Spark2dTriggerAreaComponent::m_excludedTags)
				->Field("RequiredTagsStrings", &Spark2dTriggerAreaComponent::m_requiredTagsStrings)
				->Field("ExcludedTagsStrings", &Spark2dTriggerAreaComponent::m_excludedTagsStrings);

	
			if (auto editContext = serializationContext->GetEditContext())
			{
				editContext->Class<Spark2dTriggerAreaComponent>("Spark 2D Trigger Area", "")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::Category, "spark")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					->ClassElement(AZ::Edit::ClassElements::Group, "Activation")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					->DataElement(AZ::Edit::UIHandlers::Default, &Spark2dTriggerAreaComponent::m_triggerOnce, "Trigger once", "If set, the trigger will deactivate after the first trigger event.")
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &Spark2dTriggerAreaComponent::m_activationEntityType, "Activated by", "The types of entities capable of interacting with the area trigger.")
					->EnumAttribute(Spark2dTriggerAreaComponent::ActivationEntityType::AllEntities, "All entities")
					->EnumAttribute(Spark2dTriggerAreaComponent::ActivationEntityType::SpecificEntities, "Specific entities")
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &Spark2dTriggerAreaComponent::OnActivatedByComboBoxChanged)
					->DataElement(AZ::Edit::UIHandlers::Default, &Spark2dTriggerAreaComponent::m_specificInteractEntities, "Specific entities", "List of entities that can interact with the trigger.")
					->Attribute(AZ::Edit::Attributes::Visibility, &Spark2dTriggerAreaComponent::UseSpecificEntityList)
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->ClassElement(AZ::Edit::ClassElements::Group, "Tag Filters")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					->DataElement(AZ::Edit::UIHandlers::Default, &Spark2dTriggerAreaComponent::m_requiredTagsStrings, "Required tags", "These tags are required on an entity for it to activate this Trigger")
					->DataElement(AZ::Edit::UIHandlers::Default, &Spark2dTriggerAreaComponent::m_excludedTagsStrings, "Excluded tags", "The tags exclude an entity from activating this Trigger");;
			}
		}

		AzFramework::NetworkContext* netContext = azrtti_cast<AzFramework::NetworkContext*>(context);
		if (netContext)
		{
			netContext->Class<Spark2dTriggerAreaComponent>()
				->Chunk<TriggerAreaReplicaChunk, TriggerAreaReplicaChunkDesc>()
				->RPC<TriggerAreaReplicaChunk, Spark2dTriggerAreaComponent>("OnAreaEnter", &TriggerAreaReplicaChunk::OnAreaEnter)
				->RPC<TriggerAreaReplicaChunk, Spark2dTriggerAreaComponent>("OnAreaExit", &TriggerAreaReplicaChunk::OnAreaExit);
		}
	}

	//=========================================================================
	// Constructor
	//=========================================================================
	Spark2dTriggerAreaComponent::Spark2dTriggerAreaComponent()
		: m_activationEntityType(ActivationEntityType::AllEntities)
		, m_triggerOnce(false)
		, m_replicaChunk(nullptr)
	{
	}

	void Spark2dTriggerAreaComponent::Init()
	{
		m_requiredTags.clear();
		for (const auto& requiredTag : m_requiredTagsStrings)
		{
			AddRequiredTagInternal(AZ::Crc32(requiredTag.c_str()));
		}

		m_excludedTags.clear();
		for (const auto& excludedTag : m_excludedTagsStrings)
		{
			AddExcludedTagInternal(AZ::Crc32(excludedTag.c_str()));
		}
	}

	//=========================================================================
	// Activate
	//=========================================================================
	void Spark2dTriggerAreaComponent::Activate()
	{
		if (!IsNetworkControlled())
		{
			TriggerAreaRequestsBus::Handler::BusConnect(GetEntityId());
			ProximityTriggerEventBus::Handler::BusConnect(GetEntityId());
			ShapeComponentNotificationsBus::Handler::BusConnect(GetEntityId());
			SparkTriggerAreaEntityRequestBus::Handler::BusConnect(GetEntityId());
		}
	}


	//=========================================================================
	// UpdateTriggerArea
	//=========================================================================
	void Spark2dTriggerAreaComponent::UpdateTriggerArea()
	{
		AZ::Aabb encompassingAABB;
		EBUS_EVENT_ID_RESULT(encompassingAABB, GetEntityId(), ShapeComponentRequestsBus, GetEncompassingAabb);
		m_cachedAABB = AZAabbToLyAABB(encompassingAABB);
	}

	//=========================================================================
	// Deactivate
	//=========================================================================
	void Spark2dTriggerAreaComponent::Deactivate()
	{
		SparkTriggerAreaEntityRequestBus::Handler::BusDisconnect();
		ProximityTriggerEventBus::Handler::BusDisconnect();
		ShapeComponentNotificationsBus::Handler::BusDisconnect();
		TriggerAreaRequestsBus::Handler::BusDisconnect();
		AZ::TickBus::Handler::BusDisconnect();
	}


	bool Spark2dTriggerAreaComponent::AddRequiredTagInternal(const Tag& requiredTag)
	{
		bool isTagExcluded = (m_excludedTags.end() != AZStd::find(m_excludedTags.begin(), m_excludedTags.end(), requiredTag));
		AZ_Warning("Spark2dTriggerAreaComponent", !isTagExcluded, "Required tag is already Excluded");

		if (!isTagExcluded)
		{
			m_requiredTags.push_back(requiredTag);
		}

		return !isTagExcluded;
	}

	void Spark2dTriggerAreaComponent::AddRequiredTag(const Tag& requiredTag)
	{
		if (AddRequiredTagInternal(requiredTag))
		{
			ReevaluateTagsAllEntities();
		}
	}

	void Spark2dTriggerAreaComponent::RemoveRequiredTag(const Tag& requiredTag)
	{
		const auto& requiredTagIter = AZStd::find(m_requiredTags.begin(), m_requiredTags.end(), requiredTag);
		bool isTagRequired = (m_requiredTags.end() != requiredTagIter);
		AZ_Warning("Spark2dTriggerAreaComponent", isTagRequired, "No such tag is required %i", requiredTag);

		if (isTagRequired)
		{
			m_requiredTags.erase(requiredTagIter);
			ReevaluateTagsAllEntities();
		}
	}

	bool Spark2dTriggerAreaComponent::AddExcludedTagInternal(const Tag& excludedTag)
	{
		bool isTagRequired = (m_requiredTags.end() != AZStd::find(m_requiredTags.begin(), m_requiredTags.end(), excludedTag));

		AZ_Warning("Spark2dTriggerAreaComponent", !isTagRequired, "Excluded tag is already Required");

		if (!isTagRequired)
		{
			m_excludedTags.push_back(excludedTag);
		}

		return !isTagRequired;
	}

	void Spark2dTriggerAreaComponent::AddExcludedTag(const Tag& excludedTag)
	{
		if (AddExcludedTagInternal(excludedTag))
		{
			ReevaluateTagsAllEntities();
		}
	}

	void Spark2dTriggerAreaComponent::RemoveExcludedTag(const Tag& excludedTag)
	{
		const auto& excludedTagIter = AZStd::find(m_excludedTags.begin(), m_excludedTags.end(), excludedTag);
		bool isTagExcluded = (m_excludedTags.end() != excludedTagIter);
		AZ_Warning("Spark2dTriggerAreaComponent", isTagExcluded, "No such tag is excluded %i", excludedTag);

		if (isTagExcluded)
		{
			m_excludedTags.erase(excludedTagIter);
			ReevaluateTagsAllEntities();
		}
	}

	void Spark2dTriggerAreaComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
	{
		TriggerAreaReplicaChunk* triggerAreaReplicaChunk = static_cast<TriggerAreaReplicaChunk*>(m_replicaChunk.get());

		for (AZ::u64 entityId : triggerAreaReplicaChunk->m_initialEntitiesInArea)
		{
			OnTriggerEnterImpl(AZ::EntityId(entityId));
		}

		triggerAreaReplicaChunk->m_initialEntitiesInArea.clear();
		AZ::TickBus::Handler::BusDisconnect();
	}

	void Spark2dTriggerAreaComponent::OnTransformChanged(const AZ::Transform& /*parentLocalTM*/, const AZ::Transform& /*parentWorldTM*/)
	{
		UpdateTriggerArea();
	}

	//=========================================================================
	// OnTriggerEnter
	//=========================================================================
	void Spark2dTriggerAreaComponent::OnTriggerEnter(AZ::EntityId entityId)
	{
		AZ_Error("Spark2dTriggerAreaComponent", !IsNetworkControlled(), "OnTriggerEnter being called on a proxy Spark2dTriggerAreaComponent for Entity(%s).", GetEntity()->GetName().c_str());

		if (!IsNetworkControlled())
		{
			bool result = OnTriggerEnterImpl(entityId);

			if (result && m_replicaChunk && m_replicaChunk->IsMaster())
			{
				static_cast<TriggerAreaReplicaChunk*>(m_replicaChunk.get())->OnAreaEnter(static_cast<AZ::u64>(entityId));
			}
		}
	}

	bool Spark2dTriggerAreaComponent::OnTriggerEnterImpl(AZ::EntityId entityId)
	{
		bool retVal = false;

		FilteringResult result = EntityPassesFilters(entityId);
		if (result == FilteringResult::Pass)
		{
			retVal = true;

			if (m_entitiesInside.end() == AZStd::find(m_entitiesInside.begin(), m_entitiesInside.end(), entityId))
			{
				EBUS_EVENT_ID(GetEntityId(), TriggerAreaNotificationBus, OnTriggerAreaEntered, entityId);
				EBUS_EVENT_ID(entityId, TriggerAreaEntityNotificationBus, OnEntityEnteredTriggerArea, GetEntityId());

				HandleEnter();

				m_entitiesInside.push_back(entityId);
			}
		}

		if (result == FilteringResult::FailWithPossibilityOfChange || result == FilteringResult::Pass)
		{
			TagComponentNotificationsBus::MultiHandler::BusConnect(entityId);
		}

		return retVal;
	}

	//=========================================================================
	// OnTriggerExit
	//=========================================================================
	void Spark2dTriggerAreaComponent::OnTriggerExit(AZ::EntityId entityId)
	{
		AZ_Error("Spark2dTriggerAreaComponent", !IsNetworkControlled(), "OnTriggerExit being called on a proxy Spark2dTriggerAreaComponent for Entity(%s).", GetEntity()->GetName().c_str());

		if (!IsNetworkControlled())
		{
			bool result = OnTriggerExitImpl(entityId);

			if (result && m_replicaChunk && m_replicaChunk->IsMaster())
			{
				static_cast<TriggerAreaReplicaChunk*>(m_replicaChunk.get())->OnAreaExit(static_cast<AZ::u64>(entityId));
			}
		}
	}

	bool Spark2dTriggerAreaComponent::OnTriggerExitImpl(AZ::EntityId entityId)
	{
		bool retVal = false;

		auto foundIter = AZStd::find(m_entitiesInside.begin(), m_entitiesInside.end(), entityId);
		if (foundIter != m_entitiesInside.end())
		{
			m_entitiesInside.erase(foundIter);

			retVal = true;

			EBUS_EVENT_ID(GetEntityId(), TriggerAreaNotificationBus, OnTriggerAreaExited, entityId);
			EBUS_EVENT_ID(entityId, TriggerAreaEntityNotificationBus, OnEntityExitedTriggerArea, GetEntityId());
		}

		auto excludedEntityIter = AZStd::find(m_entitiesInsideExcludedByTags.begin(), m_entitiesInsideExcludedByTags.end(), entityId);
		if (excludedEntityIter != m_entitiesInsideExcludedByTags.end())
		{
			m_entitiesInsideExcludedByTags.erase(excludedEntityIter);
		}

		TagComponentNotificationsBus::MultiHandler::BusDisconnect(entityId);

		return retVal;
	}

	//=========================================================================
	// DebugDraw
	//=========================================================================
	void Spark2dTriggerAreaComponent::DebugDraw() const
	{
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawAABB(m_cachedAABB, false, m_entitiesInside.empty() ? Col_LightGray : Col_Green, eBBD_Faceted);
	}

	//=========================================================================
	// HandleEnter
	//=========================================================================
	void Spark2dTriggerAreaComponent::HandleEnter()
	{
		if (m_entitiesInside.size() == 0) // First one in
		{
			if (m_triggerOnce)
			{
				Deactivate();
			}
		}
	}

	//=========================================================================
	// EntityPassesFilters
	//=========================================================================
	Spark2dTriggerAreaComponent::FilteringResult Spark2dTriggerAreaComponent::EntityPassesFilters(AZ::EntityId id) const
	{
		bool result = false;

		switch (m_activationEntityType)
		{
		case ActivationEntityType::AllEntities:
		{
			result = true;
			break;
		}
		case ActivationEntityType::SpecificEntities:
		{
			auto foundIter = AZStd::find(m_specificInteractEntities.begin(), m_specificInteractEntities.end(), id);
			result = foundIter != m_specificInteractEntities.end();
			break;
		}
		}

		if (!result)
		{
			return FilteringResult::FailWithoutPossibilityOfChange;
		}

		if (result)
		{
			for (const Tag& requiredTag : m_requiredTags)
			{
				result = false;
				EBUS_EVENT_ID_RESULT(result, id, TagComponentRequestBus, HasTag, requiredTag);
				if (!result)
				{
					break;
				}
			}
		}

		if (result && LmbrCentral::TagComponentRequestBus::FindFirstHandler(id))
		{
			for (const Tag& requiredTag : m_excludedTags)
			{
				result = true;
				EBUS_EVENT_ID_RESULT(result, id, TagComponentRequestBus, HasTag, requiredTag);
				result = !result;
				if (!result)
				{
					break;
				}
			}
		}

		if (!result)
		{
			return FilteringResult::FailWithPossibilityOfChange;
		}
		else
		{
			return FilteringResult::Pass;
		}
	}

	//=========================================================================
	// IsPlayer
	//=========================================================================
	bool Spark2dTriggerAreaComponent::IsPlayer(AZ::EntityId entityId)
	{
		// We don't yet have a way to create player actors in AZ::Entity/Components,
		// so check legacy code paths.
		if (IsLegacyEntityId(entityId))
		{
			const /*Cry*/ EntityId id = GetLegacyEntityId(entityId);
			IActor* actor = gEnv->pGame && gEnv->pGame->GetIGameFramework() ? gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id) : nullptr;
			if (actor && actor->GetChannelId() != kInvalidChannelId)
			{
				return true;
			}
		}

		return false;
	}

	//=========================================================================
	// IsLocalPlayer
	//=========================================================================
	bool Spark2dTriggerAreaComponent::IsLocalPlayer(AZ::EntityId entityId)
	{
		// We don't yet have a way to create player actors in AZ::Entity/Components,
		// so check legacy code paths.
		if (IsLegacyEntityId(entityId) && gEnv->pGame)
		{
			const /*Cry*/ EntityId id = GetLegacyEntityId(entityId);
			return (id == gEnv->pGame->GetClientActorId());
		}

		return false;
	}
	bool Spark2dTriggerAreaComponent::IsNetworkControlled() const
	{
		return m_replicaChunk != nullptr && m_replicaChunk->IsActive() && m_replicaChunk->IsProxy();
	}

	GridMate::ReplicaChunkPtr Spark2dTriggerAreaComponent::GetNetworkBinding()
	{
		TriggerAreaReplicaChunk* replicaChunk = GridMate::CreateReplicaChunk<TriggerAreaReplicaChunk>();
		replicaChunk->SetHandler(this);

		m_replicaChunk = replicaChunk;

		return m_replicaChunk;
	}

	void Spark2dTriggerAreaComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
	{
		chunk->SetHandler(this);

		m_replicaChunk = chunk;

		AZ::TickBus::Handler::BusConnect();
	}

	void Spark2dTriggerAreaComponent::UnbindFromNetwork()
	{
		m_replicaChunk->SetHandler(nullptr);
		m_replicaChunk = nullptr;
	}

	bool Spark2dTriggerAreaComponent::OnEntityEnterAreaRPC(AZ::u64 entityId, const GridMate::RpcContext& rpcContext)
	{
		bool allowTrigger = false;
		if (IsNetworkControlled())
		{
			OnTriggerEnterImpl(AZ::EntityId(entityId));
		}
		else if (!m_entitiesInside.empty())
		{
			allowTrigger = (m_entitiesInside.back() == AZ::EntityId(entityId));
		}

		return allowTrigger;
	}

	bool Spark2dTriggerAreaComponent::OnEntityExitAreaRPC(AZ::u64 entityId, const GridMate::RpcContext& rpcContext)
	{
		if (IsNetworkControlled())
		{
			OnTriggerExitImpl(AZ::EntityId(entityId));
		}

		return true;
	}

	bool Spark2dTriggerAreaComponent::NarrowPassCheck(const AZ::Vector3& position)
	{
		

		using namespace LmbrCentral;

		AZ::EntityId entityId = GetEntityId();
		AZ::Crc32 shapeType;
		ShapeComponentRequestsBus::EventResult(shapeType, entityId, &ShapeComponentRequests::GetShapeType);

		bool is_circle = false;
		float radius;
		AZ::Vector3 center;


		if (shapeType == AZ_CRC("Sphere", 0x55f96687))
		{
			EBUS_EVENT_ID_RESULT(radius, entityId, SphereShapeComponentRequestsBus, GetRadius);
			is_circle = true;
		}

		if (shapeType == AZ_CRC("Cylinder", 0x9b045bea))
		{
			EBUS_EVENT_ID_RESULT(radius, entityId, CylinderShapeComponentRequestsBus, GetRadius);
			is_circle = true;
		}

		if (shapeType == AZ_CRC("Capsule", 0xc268a183))
		{
			EBUS_EVENT_ID_RESULT(radius, entityId, CapsuleShapeComponentRequestsBus, GetRadius);
			is_circle = true;
		}

		if (is_circle)
		{
			EBUS_EVENT_ID_RESULT(center, entityId, AZ::TransformBus, GetWorldTranslation);

			return Distance2D(center, position) < radius;
		}
		

		bool isPointInside = false;

		//make the z equal to the one of the trigger area ( this way we kinda ignore the 3rd dimension)
		AZ::Vector3 pos;
		AZ::Vector3 new_pos;
		EBUS_EVENT_ID_RESULT(pos, entityId, AZ::TransformBus, GetWorldTranslation);
		new_pos = position;
		new_pos.SetZ(pos.GetZ() + 0.1f);

		EBUS_EVENT_ID_RESULT(isPointInside, entityId, ShapeComponentRequestsBus, IsPointInside, new_pos);

		return isPointInside;// true;
	}

	void Spark2dTriggerAreaComponent::OnShapeChanged(ShapeComponentNotifications::ShapeChangeReasons changeReason)
	{
		(void)changeReason;
		UpdateTriggerArea();
	}

	void Spark2dTriggerAreaComponent::ReevaluateTagsAllEntities()
	{
		for (const AZ::EntityId& entityInside : m_entitiesInside)
		{
			HandleTagChange(entityInside);
		}

		for (const AZ::EntityId& entityInside : m_entitiesInsideExcludedByTags)
		{
			HandleTagChange(entityInside);
		}
	}

	void Spark2dTriggerAreaComponent::HandleTagChange(const AZ::EntityId& entityId)
	{
		FilteringResult result = EntityPassesFilters(entityId);
		if (result != FilteringResult::Pass)
		{
			OnTriggerExit(entityId);
			if (result == FilteringResult::FailWithPossibilityOfChange)
			{
				TagComponentNotificationsBus::MultiHandler::BusConnect(entityId);
				m_entitiesInsideExcludedByTags.push_back(entityId);
			}
		}
		else
		{
			OnTriggerEnter(entityId);
			auto foundIter = AZStd::find(m_entitiesInsideExcludedByTags.begin(), m_entitiesInsideExcludedByTags.end(), entityId);
			if (foundIter != m_entitiesInsideExcludedByTags.end())
			{
				m_entitiesInsideExcludedByTags.erase(foundIter);
			}
		}
	}

	void Spark2dTriggerAreaComponent::OnTagAdded(const Tag& tagAdded)
	{
		AZ::EntityId tagAddedToEntity = *(TagComponentNotificationsBus::GetCurrentBusId());
		HandleTagChange(tagAddedToEntity);
	}

	void Spark2dTriggerAreaComponent::OnTagRemoved(const Tag& tagRemoved)
	{
		AZ::EntityId tagRemovedFromEntity = *(TagComponentNotificationsBus::GetCurrentBusId());
		HandleTagChange(tagRemovedFromEntity);
	}

} 
