
#include "spark_precompiled.h"
#include "DynamicSliceWrapper.h"



using namespace spark;
using namespace AzFramework;


//DynamicSliceWrapper implementation

bool DynamicSliceWrapper::IsReady() const {
	return m_id.IsValid();
}
AZ::EntityId DynamicSliceWrapper::GetId() const {
	return m_id;
}
AZ::Data::AssetId DynamicSliceWrapper::GetAssetId() const {
	return m_asset.GetId();
}

void DynamicSliceWrapper::Load()
{
	if (m_state != waiting) 
	{
		AZ_Error("DynamicSliceWrapper", false, "called Load, but asset is not specified");
		return;
	}
	//AZ_Printf(0, "DynamicSliceWrapper::Load()");
	EBUS_EVENT_RESULT(m_ticket, AzFramework::GameEntityContextRequestBus, InstantiateDynamicSlice, m_asset, transform, nullptr);
	AzFramework::SliceInstantiationResultBus::Handler::BusConnect(m_ticket);
	m_state = loading;
}
bool DynamicSliceWrapper::Loaded() const
{
	return m_state == loaded;
}
DynamicSliceWrapper::DynamicSliceWrapper(const DynamicSliceAsset &asset) {
	m_state = waiting;
	m_asset = asset;
}

spark::DynamicSliceWrapper::~DynamicSliceWrapper()
{
	if(m_state == loading)AzFramework::SliceInstantiationResultBus::Handler::BusDisconnect();
}



void DynamicSliceWrapper::OnSliceInstantiated(const AZ::Data::AssetId& sliceAssetId, const AZ::SliceComponent::SliceInstanceAddress& instance) {
	m_id = (*(instance.second->GetInstantiated()->m_entities.begin()))->GetId();
	AzFramework::SliceInstantiationResultBus::Handler::BusDisconnect();

	AZ::Entity *e = (*(instance.second->GetInstantiated()->m_entities.begin()));

	m_state = loaded;
	//AZ_Printf(0, "DynamicSliceWrapper::OnSliceInstantiated  entity's state is %d", e->GetState());
	if (onInstantiated)onInstantiated(e);
}

void DynamicSliceWrapper::OnSlicePreInstantiate(const AZ::Data::AssetId& sliceAssetId, const AZ::SliceComponent::SliceInstanceAddress& instance) {

	AZ::Entity *e = (*(instance.second->GetInstantiated()->m_entities.begin()));

	
	//AZ_Printf(0, "DynamicSliceWrapper::OnSlicePreInstantiate  entity's state is %d", e->GetState());

	if (onPreInstantiate)onPreInstantiate(e);
}

void DynamicSliceWrapper::OnSliceInstantiationFailed(const AZ::Data::AssetId& /*sliceAssetId*/)
{ 
	AZ_Printf(0, "DynamicSliceWrapper::OnSliceInstantiationFailed");
	AzFramework::SliceInstantiationResultBus::Handler::BusDisconnect();
	m_state = loaded;
}

void DynamicSliceWrapper::OnSliceInstantiationFailedOrCanceled(const AZ::Data::AssetId& /*sliceAssetId*/, bool /*canceled*/) 
{
	AZ_Printf(0, "DynamicSliceWrapper::OnSliceInstantiationFailedOrCanceled");
	AzFramework::SliceInstantiationResultBus::Handler::BusDisconnect();
	m_state = loaded;
}