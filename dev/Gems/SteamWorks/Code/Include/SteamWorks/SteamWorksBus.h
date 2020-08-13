
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/RTTI/TypeInfo.h>

// #pragma warning(push)
// #pragma warning(disable:4996)
#include <steam/steamclientpublic.h>
// #pragma warning(pop)

namespace AZ
{
	AZ_TYPE_INFO_SPECIALIZE(CSteamID, "8D2AC430-BC34-4C43-AD1E-4EBAEBF5CB14");
};

namespace SteamWorks
{
	struct AuthTokenResult
	{
		HAuthTicket ticket;
		AZStd::vector<char> token;
	};
	class SteamWorksRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		// Put your public methods here
		virtual CSteamID GetSteamID() const = 0;
		virtual AZStd::string GetSteamName() const = 0;

		virtual AuthTokenResult GetAuthToken() = 0;
		virtual AuthTokenResult GetServerAuthToken() = 0;
		virtual EBeginAuthSessionResult ConfirmAuthToken(const AZStd::vector<char> &, const CSteamID &steamID) = 0;

	};
	using SteamWorksRequestBus = AZ::EBus<SteamWorksRequests>;


	class SteamFriendsNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		void OnGameOverlayActivated (bool activated) {}
	};
	using SteamFriendsNotificationBus = AZ::EBus<SteamFriendsNotifications>;

} // namespace SteamWorks

// hasher
template<>
struct AZStd::hash<CSteamID>
{
	AZ_FORCE_INLINE AZStd::size_t operator()(const CSteamID& value) const
	{
		return AZStd::hash<uint64_t>()(value.ConvertToUint64());
	}
};

// marshaler
template<>
class GridMate::Marshaler<CSteamID>
{
public:
	typedef CSteamID DataType;

	AZ_FORCE_INLINE void Marshal(WriteBuffer& wb, const DataType& value) const
	{
		Marshaler<uint64_t> intMarshaler;

		intMarshaler.Marshal(wb, value.ConvertToUint64());
	}
	AZ_FORCE_INLINE void Unmarshal(DataType& value, ReadBuffer& rb) const
	{
		Marshaler<uint64_t> intMarshaler;
		uint64_t id;
		intMarshaler.Unmarshal(id, rb);

		value.SetFromUint64(id);
	}
};

