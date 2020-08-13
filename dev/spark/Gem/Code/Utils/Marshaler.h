#pragma once

// common marshalers
// entity id, in network for some reason
#include <AzFramework/Network/EntityIdMarshaler.h>
// containers include strings because strings are just containers
#include <GridMate/Serialize/ContainerMarshal.h>
// all the remaining AZStd stuff like pairs and maths
#include <GridMate/Serialize/UtilityMarshal.h>
