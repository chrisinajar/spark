#pragma once


#include "Busses/DynamicSliceManagerBus.h"
#include <AzCore/std/containers/map.h>



namespace spark
{

	class DynamicSliceManagerComponent
		: public AZ::Component
		, public DynamicSliceManagerRequestBus::Handler
	{
	public:
		AZ_COMPONENT(DynamicSliceManagerComponent, "{D74CAF06-5D2E-4C40-A546-880F5BA6E1EF}");

		static void Reflect(AZ::ReflectContext* context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

	protected:

		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;
		////////////////////////////////////////////////////////////////////////

		DynamicSliceAsset GetDynamicSliceAsset(DynamicSliceTypeId id);
		DynamicSliceAssetId GetDynamicSliceAssetId(DynamicSliceTypeId id);

		AzFramework::SliceInstantiationTicket SpawnDynamicSlice(DynamicSliceTypeId id);

	protected:
		AZStd::map<DynamicSliceTypeId, AZStd::pair<DynamicSliceAssetId, DynamicSliceAsset> > m_slices_cache;
	};


}