#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <AzFramework/Network/NetBindable.h>


namespace spark
{
	class ParticleSystemNetSyncComponent
		: public AZ::Component
		, public AzFramework::NetBindable
	{
	public:
		AZ_COMPONENT(ParticleSystemNetSyncComponent, "{36572005-2097-4925-93D6-46C47AE1967A}", AzFramework::NetBindable);

		static void Reflect(AZ::ReflectContext* context);

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		GridMate::ReplicaChunkPtr GetNetworkBinding();
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk);
		void UnbindFromNetwork();


		void OnNewParticleSystemTypeId(const AZStd::string& value, const GridMate::TimeContext& tc);
		void OnNewSizeScale(const float& value, const GridMate::TimeContext& tc);
		void OnNewParticleVisibility(const bool& value, const GridMate::TimeContext& tc);

		void SetupParticleSystem();
		void UpdateParticleSystemValues();

		AZStd::string m_particleSystemTypeId;
		float m_sizeScale = 1.0f;
		bool m_visibility = true;
	private:
		
		bool m_isAuthoritative = false;

		GridMate::ReplicaChunkPtr m_replicaChunk;
	};
}