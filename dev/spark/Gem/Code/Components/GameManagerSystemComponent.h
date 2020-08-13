#pragma once


#include "Busses/GameManagerBus.h"
#include <AzCore/std/containers/map.h>
#include <AzCore/std/containers/vector.h>
#include <AZCore/JSON/document.h>
#include <AzCore/Component/TickBus.h>


namespace spark
{

	class GameManagerSystemComponent
		: public AZ::Component,
		public GameManagerSystemRequestBus::Handler,
		protected GameManagerNotificationBus::Handler,
		private AZ::SystemTickBus::Handler
	{
	public:
		AZ_COMPONENT(GameManagerSystemComponent, "{1DE4DBBF-9443-4F83-BC1B-528E256DC07A}");

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

		bool    LoadGameFile(AZStd::string filename);
		bool    LoadGameMode(AZStd::string gamemodeName);
		void    PlayGame();

		void    ExecuteConsoleCommand(AZStd::string cmd);

		void OnGameManagerActivated(AZ::Entity* gameManager);

	private:

		void CreateGameManager();
		// SystemTickBus
        void OnSystemTick() override;
		int m_tickCounter;

		rapidjson::Document m_gameDocument;
		
		static AZStd::vector<AZStd::string> s_filenames, s_directories;
	};


}