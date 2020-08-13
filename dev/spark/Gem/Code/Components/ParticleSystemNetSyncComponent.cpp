
#include "spark_precompiled.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>

#include "Utils/Log.h"



#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/RemoteProcedureCall.h>

#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <GridMate/Serialize/CompressionMarshal.h>
#include "Utils/Marshaler.h"

#include "ParticleSystemNetSyncComponent.h"
#include <LmbrCentral/Rendering/ParticleComponentBus.h>

using namespace GridMate;

namespace spark
{

	class ParticleSystemNetSyncComponentChunk
		: public GridMate::ReplicaChunkBase
	{
	public:
		GM_CLASS_ALLOCATOR(ParticleSystemNetSyncComponentChunk);

		ParticleSystemNetSyncComponentChunk() 
			: m_particleSystemTypeId("type")
			, m_sizeScale("size")
			, m_visibility("visibility")
		{}

		bool IsReplicaMigratable() override
		{
			return true;
		}

		static const char* GetChunkName()
		{
			return "ParticleSystemNetSyncComponentChunk";
		}

		GridMate::DataSet<AZStd::string>::BindInterface<ParticleSystemNetSyncComponent, &ParticleSystemNetSyncComponent::OnNewParticleSystemTypeId> m_particleSystemTypeId;
		GridMate::DataSet<float>::BindInterface<ParticleSystemNetSyncComponent, &ParticleSystemNetSyncComponent::OnNewSizeScale> m_sizeScale;
		GridMate::DataSet<bool>::BindInterface<ParticleSystemNetSyncComponent, &ParticleSystemNetSyncComponent::OnNewParticleVisibility> m_visibility;
	};



	void ParticleSystemNetSyncComponent::Reflect(AZ::ReflectContext* reflection)
	{
		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			serializationContext->Class<ParticleSystemNetSyncComponent, AZ::Component>()
				->Version(1);

			if (auto editContext = serializationContext->GetEditContext())
			{
				editContext->Class<ParticleSystemNetSyncComponent>("ParticleSystemNetSyncComponent", "")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::Category, "spark")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
			}
		}
		AzFramework::NetworkContext* netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection);
		if (netContext)
		{
			netContext->Class<ParticleSystemNetSyncComponent>()
				->Chunk<ParticleSystemNetSyncComponentChunk>()
				->Field("type", &ParticleSystemNetSyncComponentChunk::m_particleSystemTypeId)
				->Field("size", &ParticleSystemNetSyncComponentChunk::m_sizeScale)
				->Field("visibility", &ParticleSystemNetSyncComponentChunk::m_visibility)
				;
		}
	}

	void ParticleSystemNetSyncComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("ParticleSystemNetSyncService"));
	}

	void ParticleSystemNetSyncComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("ParticleSystemNetSyncService"));
	}

	void ParticleSystemNetSyncComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		required.push_back(AZ_CRC("ParticleService", 0x725d4a5d));
	}

	void ParticleSystemNetSyncComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		(void)dependent;
	}

	void ParticleSystemNetSyncComponent::Init()
	{
		//sLOG("ParticleSystemNetSyncComponent::Init()");
	}

	void ParticleSystemNetSyncComponent::Activate()
	{
		//sLOG("ParticleSystemNetSyncComponent::Activate()");

		SetupParticleSystem();
	}

	void ParticleSystemNetSyncComponent::Deactivate()
	{
	}

	//NetBindable
	GridMate::ReplicaChunkPtr ParticleSystemNetSyncComponent::GetNetworkBinding()
	{
		//sLOG("ParticleSystemNetSyncComponent::GetNetworkBinding()");

		auto replicaChunk = GridMate::CreateReplicaChunk<ParticleSystemNetSyncComponentChunk>();
		replicaChunk->SetHandler(this);
		m_replicaChunk = replicaChunk;

		if (ParticleSystemNetSyncComponentChunk* chunk = static_cast<ParticleSystemNetSyncComponentChunk*>(m_replicaChunk.get()))
		{
			chunk->m_particleSystemTypeId.Set(m_particleSystemTypeId);
			chunk->m_sizeScale.Set(m_sizeScale);
			chunk->m_visibility.Set(m_visibility);
		}
		return m_replicaChunk;
	}

	void ParticleSystemNetSyncComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
	{
		//sLOG("ParticleSystemNetSyncComponent::SetNetworkBinding()");

		chunk->SetHandler(this);
		m_replicaChunk = chunk;
		
		if (ParticleSystemNetSyncComponentChunk* chunk = static_cast<ParticleSystemNetSyncComponentChunk*>(m_replicaChunk.get()))
		{
			m_particleSystemTypeId = chunk->m_particleSystemTypeId.Get();
			m_sizeScale = chunk->m_sizeScale.Get();
			m_visibility = chunk->m_visibility.Get();
			SetupParticleSystem();
		}
	}

	void ParticleSystemNetSyncComponent::UnbindFromNetwork()
	{
		//sLOG("ParticleSystemNetSyncComponent::GetNetworkBinding()");

		if (m_replicaChunk)
		{
			m_replicaChunk->SetHandler(nullptr);
			m_replicaChunk = nullptr;
		}
	}


	void ParticleSystemNetSyncComponent::OnNewParticleSystemTypeId(const AZStd::string& value, const GridMate::TimeContext& tc)
	{
		m_particleSystemTypeId = value;
		SetupParticleSystem();
	}
	void ParticleSystemNetSyncComponent::OnNewSizeScale(const float& value, const GridMate::TimeContext& tc)
	{
		m_sizeScale = value;
		SetupParticleSystem();
	}
	void ParticleSystemNetSyncComponent::OnNewParticleVisibility(const bool& value, const GridMate::TimeContext& tc)
	{
		m_visibility = true;
		SetupParticleSystem();
	}

	void ParticleSystemNetSyncComponent::SetupParticleSystem()
	{
		//sLOG("SetupParticleSystem  type is : " + m_particleSystemTypeId);
		LmbrCentral::ParticleEmitterSettings particleSettings;
		particleSettings.m_selectedEmitter = m_particleSystemTypeId;
		particleSettings.m_sizeScale = m_sizeScale;
		particleSettings.m_visible = m_visibility;
		EBUS_EVENT_ID(GetEntityId(), LmbrCentral::ParticleComponentRequestBus, SetupEmitter, particleSettings.m_selectedEmitter, particleSettings);
	}

	void ParticleSystemNetSyncComponent::UpdateParticleSystemValues()
	{

	}
}
