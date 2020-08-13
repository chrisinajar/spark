

#include "spark_precompiled.h"

#include "TagNetSyncComponent.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>

#include "Utils/Marshaler.h"

using namespace spark;


class TagNetSyncReplicaChunk : public GridMate::ReplicaChunkBase
{
public:
    AZ_CLASS_ALLOCATOR(TagNetSyncReplicaChunk, AZ::SystemAllocator, 0);

    static const char* GetChunkName() { return "TagNetSyncReplicaChunk"; }

       TagNetSyncReplicaChunk()
       : m_tags("Tags")
    {
    }

    bool IsReplicaMigratable()
    {
        return true;
    }

	GridMate::DataSet<LmbrCentral::Tags>::BindInterface<TagNetSyncComponent, &TagNetSyncComponent::OnNewTags> m_tags;
};

void TagNetSyncComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<TagNetSyncComponent, AzFramework::NetBindable, AZ::Component>()
			->Version(1)
			;

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<TagNetSyncComponent>("TagNetSyncComponent", "Net sync TagComponent")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "Gameplay")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				;
		}
	} 
	if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection))
	{
		netContext->Class<TagNetSyncComponent>()
			->Chunk<TagNetSyncReplicaChunk>()
			->Field("Tags", &TagNetSyncReplicaChunk::m_tags)
			;
	}
}

void TagNetSyncComponent::Init()
{
}

void TagNetSyncComponent::Activate()
{
	AZ_Printf(0, "TagNetSyncComponent::Activate() called");

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
	    AZ::TickBus::Handler::BusConnect();
    	LmbrCentral::TagComponentNotificationsBus::Handler::BusConnect(GetEntityId());
    	m_needsUpdate = true;
	}
}

void TagNetSyncComponent::Deactivate()
{
    AZ::TickBus::Handler::BusDisconnect();
	LmbrCentral::TagComponentNotificationsBus::Handler::BusDisconnect();
}

GridMate::ReplicaChunkPtr TagNetSyncComponent::GetNetworkBinding()
{
	AZ_Printf(0, "TagNetSyncComponent::GetNetworkBinding() called");
    auto replicaChunk = GridMate::CreateReplicaChunk<TagNetSyncReplicaChunk>();
    replicaChunk->SetHandler(this);
    m_replicaChunk = replicaChunk;

	replicaChunk->m_tags.Set(m_tags);

    return m_replicaChunk;
}

void TagNetSyncComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
{
	AZ_Printf(0, "TagNetSyncComponent::SetNetworkBinding() called");
	chunk->SetHandler(this);
	m_replicaChunk = chunk;

    TagNetSyncReplicaChunk* replicaChunk = static_cast<TagNetSyncReplicaChunk*>(m_replicaChunk.get());

	m_tags = replicaChunk->m_tags.Get();
	AZ_Printf(0, "TagNetSyncComponent::SetNetworkBinding() has tag count of %d", m_tags.size());
	SyncTags();
}

void TagNetSyncComponent::UnbindFromNetwork()
{
    m_replicaChunk->SetHandler(nullptr);
    m_replicaChunk = nullptr;
}

void TagNetSyncComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	// debounce
	if (m_needsUpdate && m_replicaChunk)
	{
		AZ_Printf(0, "TagNetSyncComponent::m_needsUpdate needs to update!");
		m_needsUpdate = false;
    	TagNetSyncReplicaChunk* replicaChunk = static_cast<TagNetSyncReplicaChunk*>(m_replicaChunk.get());

    	EBUS_EVENT_ID_RESULT(m_tags, GetEntityId(), LmbrCentral::TagComponentRequestBus, GetTags);

		replicaChunk->m_tags.Set(m_tags);
	}
}

void TagNetSyncComponent::OnNewTags (const LmbrCentral::Tags &newTags, const GridMate::TimeContext&)
{
	AZ_Printf(0, "TagNetSyncComponent::OnNewTags() called");
	m_tags = newTags;

	SyncTags();
}

void TagNetSyncComponent::SyncTags ()
{
	LmbrCentral::Tags tags;
	EBUS_EVENT_ID_RESULT(tags, GetEntityId(), LmbrCentral::TagComponentRequestBus, GetTags);

	AZ_Printf(0, "TagNetSyncComponent::SetNetworkBinding() has tag count of %d / %d", tags.size(), m_tags.size());

	for (const LmbrCentral::Tag& tag : m_tags)
	{
		bool hasTag = false;
		for (const LmbrCentral::Tag& existingTag : tags)
		{
			if (existingTag == tag)
			{
				hasTag = true;
				break;
			}
		}
		if (!hasTag)
		{
			AZ_Printf(0, "TagNetSyncComponent adding this tag to the entity! 0x%08x", tag);
			EBUS_EVENT_ID(GetEntityId(), LmbrCentral::TagComponentRequestBus, AddTag, tag);
		}
	}

	for (const LmbrCentral::Tag& existingTag : tags)
	{
		bool shouldHaveTag = false;
		for (const LmbrCentral::Tag& tag : m_tags)
		{
			if (existingTag == tag)
			{
				shouldHaveTag = true;
				break;
			}
		}
		if (!shouldHaveTag)
		{
			AZ_Printf(0, "TagNetSyncComponent removing this tag to the entity! 0x%08x", existingTag);
			EBUS_EVENT_ID(GetEntityId(), LmbrCentral::TagComponentRequestBus, RemoveTag, existingTag);
		}
	}
}

void TagNetSyncComponent::OnTagAdded(const LmbrCentral::Tag &tag)
{
	m_needsUpdate = true;
}

void TagNetSyncComponent::OnTagRemoved(const LmbrCentral::Tag &tag)
{
	m_needsUpdate = true;
}
