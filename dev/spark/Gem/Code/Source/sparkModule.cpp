
#include "spark_precompiled.h"
#include <platform_impl.h>

#include <AzCore/Memory/SystemAllocator.h>

#include "sparkSystemComponent.h"

#include <IGem.h>

#include "Components/CameraControllerComponent.h"
#include "Components/UnitComponent.h"
#include "Components/UnitNavigationComponent.h"
#include "Components/UnitAbilityComponent.h"
#include "Components/UnitAttackComponent.h"
#include "Components/UnitNetSyncComponent.h"
#include "Components/GameNetSyncComponent.h"
#include "Components/AbilityComponent.h"
#include "Components/ProjectileControllerComponent.h"
#include "Components/InputMapperSystemComponent.h"
#include "Components/GameManagerSystemComponent.h"
#include "Components/GameManagerComponent.h"
#include "Components/ProjectileManagerSystemComponent.h"
#include "Components/VariableManagerComponent.h"
#include "Components/VariableHolderComponent.h"
#include "Components/ShopComponent.h"
#include "Components/SettingsSystemComponent.h"
#include "Components/NavigationManagerComponent.h"
#include "Components/StaticDataComponent.h"
#include "Components/TimerSystemComponent.h"
#include "Components/ModifierComponent.h"
#include "Components/DebugRenderSystemComponent.h"
#include "Components/AudioSystemComponent.h"
#include "Components/ChatComponent.h"
#include "Components/LocalizationSystemComponent.h"
#include "Components/GamePlayerComponent.h"
#include "Components/GamePlayerProxyComponent.h"
#include "Components/PlayerIdentitySystemComponent.h"
#include "Components/NavigationAreaComponent.h"
#include "Components/TriggerAreaComponent.h"
#include "Components/DynamicSliceManagerComponent.h"
#include "Components/TagNetSyncComponent.h"
#include "Components/ParticleSystemNetSyncComponent.h"

// force compile of equ8 components by including their C files from within this module
// use server when dedicated server, and otherwise force client
// auti-cheat isn't supports with in-client hosting. that's not a u, it's an upside down n
#if defined(DEDICATED_SERVER)
#include "equ8/sdk/cxx_bindings/equ8_session_manager/session_manager.c"
#else
#include "equ8/sdk/cxx_bindings/equ8_client/client.c"
#endif

namespace spark
{
	class sparkModule
		: public CryHooksModule
	{
	public:
		AZ_RTTI(sparkModule, "{B41AD8FE-0A29-49C5-AF9B-21D36FE972DC}", CryHooksModule);
		AZ_CLASS_ALLOCATOR(sparkModule, AZ::SystemAllocator, 0);

		sparkModule()
			: CryHooksModule()
		{
			// equ8
			volatile equ8_err_t equ8_rc;
			// change this on release and stuff
			// probably get it from a define instead.......... later.
			int production_mode = 0;

#if defined(DEDICATED_SERVER)
			if(!EQU8_SUCCESS(equ8_rc = equ8_sm_initialize("../spark/equ8_server", production_mode)))
#else
			if(!EQU8_SUCCESS(equ8_rc = equ8_client_initialize("../spark/equ8_client", production_mode)))
#endif
			{
				AZ_Printf(0, "EQU8 init failed: %x", equ8_rc);
				// really should exit now, do not load anything!
				gEnv->pConsole->ExecuteString("quit"); // this actually throws a segfault but fucking whatever
			}

			// Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
			m_descriptors.insert(m_descriptors.end(), {
				sparkSystemComponent::CreateDescriptor(),
				SettingsSystemComponent::CreateDescriptor(),
				CameraControllerComponent::CreateDescriptor(),
				UnitComponent::CreateDescriptor(),
				UnitNavigationComponent::CreateDescriptor(),
				AbilityComponent::CreateDescriptor(),
				UnitAbilityComponent::CreateDescriptor(),
				UnitAttackComponent::CreateDescriptor(),
				ProjectileControllerComponent::CreateDescriptor(),
				GameManagerSystemComponent::CreateDescriptor(),
				InputMapperSystemComponent::CreateDescriptor(),
				ProjectileManagerSystemComponent::CreateDescriptor(),
				VariableManagerComponent::CreateDescriptor(),
				VariableHolderComponent::CreateDescriptor(),
				GameNetSyncComponent::CreateDescriptor(),
				UnitNetSyncComponent::CreateDescriptor(),
				GameManagerComponent::CreateDescriptor(),
				ShopComponent::CreateDescriptor(),
				NavigationManagerComponent::CreateDescriptor(),
				StaticDataComponent::CreateDescriptor(),
				TimerSystemComponent::CreateDescriptor(),
				ModifierComponent::CreateDescriptor(),
				DebugRenderSystemComponent::CreateDescriptor(),
				AudioSystemComponent::CreateDescriptor(),
				SparkChatComponent::CreateDescriptor(),
				LocalizationSystemComponent::CreateDescriptor(),
				GamePlayerComponent::CreateDescriptor(),
				GamePlayerProxyComponent::CreateDescriptor(),
				PlayerIdentitySystemComponent::CreateDescriptor(),
				NavigationAreaComponent::CreateDescriptor(),
				Spark2dTriggerAreaComponent::CreateDescriptor(),
				DynamicSliceManagerComponent::CreateDescriptor(),
				TagNetSyncComponent::CreateDescriptor(),
				ParticleSystemNetSyncComponent::CreateDescriptor(),
			});
		}

		~sparkModule()
		{
#if defined(DEDICATED_SERVER)
			equ8_sm_deinitialize();
#else
			equ8_client_deinitialize();
#endif
			// m_cvars.UnregisterCVars();
		}

        void OnCrySystemInitialized(ISystem& system, const SSystemInitParams& systemInitParams)
        {
            CryHooksModule::OnCrySystemInitialized(system,systemInitParams);
        }

		/**
		 * Add required SystemComponents to the SystemEntity.
		 */
		AZ::ComponentTypeList GetRequiredSystemComponents() const override
		{
			return AZ::ComponentTypeList{
				azrtti_typeid<sparkSystemComponent>(),
				azrtti_typeid<SettingsSystemComponent>(),
				azrtti_typeid<GameManagerSystemComponent>(),
				azrtti_typeid<InputMapperSystemComponent>(),
				azrtti_typeid<TimerSystemComponent>(),
				azrtti_typeid<DebugRenderSystemComponent>(),
				azrtti_typeid<AudioSystemComponent>(),
				azrtti_typeid<LocalizationSystemComponent>(),
				azrtti_typeid<PlayerIdentitySystemComponent>(),
				azrtti_typeid<DynamicSliceManagerComponent>(),
			};
		}
	};
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(spark_46b9d586131f46f7b8d0286211becf81, spark::sparkModule)
