#pragma once

#include <AzCore/Component/ComponentBus.h>

class ConsoleNotifications
	: public AZ::EBusTraits
{
public:
	//return true to consume the command, false to propagate it to lumberyard
	virtual bool OnCommandFilter(const char* command) {return false;};
};
using ConsoleNotificationBus = AZ::EBus<ConsoleNotifications>;

