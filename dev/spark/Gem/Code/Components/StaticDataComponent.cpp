#include "spark_precompiled.h"

#include "StaticDataComponent.h"

#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <AzFramework/Network/NetworkContext.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Serialize/ContainerMarshal.h>
// NetQuery
#include <AzFramework/Network/NetBindingHandlerBus.h>

#include <AzCore/JSON/stringbuffer.h>
#include <AzCore/JSON/writer.h>

#include <sstream>

#include "Utils/JsonUtils.h"

using namespace spark;


class StaticDataReplicaChunk : public GridMate::ReplicaChunkBase
{
public:
    AZ_CLASS_ALLOCATOR(StaticDataReplicaChunk, AZ::SystemAllocator, 0);

    static const char* GetChunkName() { return "StaticDataReplicaChunk"; }

       StaticDataReplicaChunk()
        : m_jsonString("Json String")
    {
    }

    bool IsReplicaMigratable()
    {
        return true;
    }

    GridMate::DataSet<AZStd::string>::BindInterface<StaticDataComponent, &StaticDataComponent::OnNewJsonString> m_jsonString;
};

void StaticDataComponent::Reflect(AZ::ReflectContext* context)
{

	if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
    {
		serialize->Class<StaticDataComponent, AzFramework::NetBindable, AZ::Component>()
			->Version(1)
			//->Field("range",&StaticDataComponent::m_range)
            ;

        if (AZ::EditContext* ec = serialize->GetEditContext())
        {
			ec->Class<StaticDataComponent>("StaticDataComponent", "")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				//->DataElement(nullptr, &StaticDataComponent::m_range, "range", "");
                ;
        }
    }

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
	{
		behaviorContext->EBus<StaticDataRequestBus>("StaticDataRequestBus")
			->Event("GetJsonString", &StaticDataRequestBus::Events::GetJsonString)
			->Event("GetValue", &StaticDataRequestBus::Events::GetValue)
			->Event("GetSpecialValue", &StaticDataRequestBus::Events::GetSpecialValue)
			->Event("GetSpecialValueLevel", &StaticDataRequestBus::Events::GetSpecialValueLevel)
			;
	}

	if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(context))
	{
		netContext->Class<StaticDataComponent>()
			->Chunk<StaticDataReplicaChunk>()
			->Field("Json String", &StaticDataReplicaChunk::m_jsonString)
			;
	}
}
void StaticDataComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
    provided.push_back(AZ_CRC("StaticDataService"));
}

void StaticDataComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
    incompatible.push_back(AZ_CRC("StaticDataService"));
}

void StaticDataComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
    (void)required;
}

void StaticDataComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{
    (void)dependent;
}

void StaticDataComponent::Init() 
{

}

void StaticDataComponent::Activate() 
{
	StaticDataRequestBus::Handler::BusConnect(GetEntityId());
}

void StaticDataComponent::Deactivate() 
{
	StaticDataRequestBus::Handler::BusDisconnect();
}



GridMate::ReplicaChunkPtr StaticDataComponent::GetNetworkBinding()
{
	auto replicaChunk = GridMate::CreateReplicaChunk<StaticDataReplicaChunk>();
	replicaChunk->SetHandler(this);
	m_replicaChunk = replicaChunk;

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		replicaChunk->m_jsonString.Set(m_jsonString);
	}
	else
	{
		SetJson(replicaChunk->m_jsonString.Get());
	}

	return m_replicaChunk;
}

void StaticDataComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
{
	chunk->SetHandler(this);
	m_replicaChunk = chunk;

	StaticDataReplicaChunk* staticDataChunk = static_cast<StaticDataReplicaChunk*>(m_replicaChunk.get());

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		staticDataChunk->m_jsonString.Set(m_jsonString);
	}
	else
	{
		SetJson(staticDataChunk->m_jsonString.Get());
	}
}

void StaticDataComponent::UnbindFromNetwork()
{
	m_replicaChunk->SetHandler(nullptr);
	m_replicaChunk = nullptr;
}

void StaticDataComponent::OnNewJsonString (const AZStd::string& jsonString, const GridMate::TimeContext& tc)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		SetJson(jsonString);
	}
}

AZStd::string StaticDataComponent::GetJsonString()
{
	return m_jsonString;
}

AZStd::string spark::StaticDataComponent::GetValue(AZStd::string id)
{
	if (m_json.IsObject() && m_json.HasMember(id.c_str()))
	{
		return JsonUtils::ToString(m_json[id.c_str()]);
	}
	return "";
}

void StaticDataComponent::SetJson(const AZStd::string& jsonString)
{
	m_jsonString = jsonString;
	m_json.Parse(jsonString.c_str());

	AZ_Printf(0, "StaticDataComponent::SetJson String %s", m_jsonString.c_str());

	if (m_replicaChunk && AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		StaticDataReplicaChunk* staticDataChunk = static_cast<StaticDataReplicaChunk*>(m_replicaChunk.get());
		staticDataChunk->m_jsonString.Set(m_jsonString);
	}
}
void StaticDataComponent::SetJson(const rapidjson::Value& jsonValue)
{
	m_json.CopyFrom(jsonValue, m_json.GetAllocator());
	m_jsonString = JsonUtils::ToString(m_json);

	AZ_Printf(0, "StaticDataComponent::SetJson Value %s", m_jsonString.c_str());

	if (m_replicaChunk && AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		StaticDataReplicaChunk* staticDataChunk = static_cast<StaticDataReplicaChunk*>(m_replicaChunk.get());
		staticDataChunk->m_jsonString.Set(m_jsonString);
	}
}

AZStd::string StaticDataComponent::GetSpecialValue(AZStd::string id)
{
	if (!m_json.IsObject() || !m_json.HasMember("special-values"))return "";

	if (m_json["special-values"].HasMember(id.c_str()))
	{
		auto &value = m_json["special-values"][id.c_str()];
		
		if(!value.IsObject() && !value.IsArray())
		{
			return JsonUtils::ToString(value);
		}
	}
	return "";
}


AZStd::string StaticDataComponent::GetSpecialValueLevel(AZStd::string id, int level)
{
	std::string value;
	std::istringstream  stream(GetSpecialValue(id).c_str());
	level=std::max(1,level);//at least one iteration
    while (stream && level--) {
        stream >> value;
    }

    return AZStd::string(value.c_str());
}

const rapidjson::Value* spark::StaticDataComponent::GetJson() const
{
	return &m_json;
}

const rapidjson::Document& spark::StaticDataComponent::GetJsonReference() const
{
	return m_json;
}
