#pragma once

#include <AzCore/Component/Component.h>
#include <AzFramework/Network/NetBindable.h>
#include <AzCore/JSON/document.h>

#include "Busses/StaticDataBus.h"

namespace spark
{
	class StaticDataComponent
		: public AZ::Component
		, public AzFramework::NetBindable
		, protected StaticDataRequestBus::Handler
	{
	public:
		AZ_COMPONENT(StaticDataComponent, "{F100B023-987A-43CC-AB09-60C3316D193B}", AzFramework::NetBindable);

		static void Reflect(AZ::ReflectContext* context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
	public:

		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;
		////////////////////////////////////////////////////////////////////////

		AZStd::string GetJsonString();
		AZStd::string GetValue(AZStd::string);//get the values in the root
		AZStd::string GetSpecialValue(AZStd::string);//get the values in the "special-values" object (all the string)
		AZStd::string GetSpecialValueLevel(AZStd::string,int level);//get the values according to the level in the "special-values" object, starting from 1 (eg: requesting level 2 of a variable having "10 20 30 40", returns 20)   

		const rapidjson::Value* GetJson() const;
		const rapidjson::Document& GetJsonReference() const;

		void SetJson(const AZStd::string& jsonString);
		void SetJson(const rapidjson::Value& jsonValue);

		GridMate::ReplicaChunkPtr GetNetworkBinding();
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk);
		void UnbindFromNetwork();
		void OnNewJsonString (const AZStd::string& jsonString, const GridMate::TimeContext& tc);

	private:
		GridMate::ReplicaChunkPtr m_replicaChunk;

		AZStd::string m_jsonString;
		rapidjson::Document m_json;

	};

}
