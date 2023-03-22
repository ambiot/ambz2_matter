#include "matter_interface.h"
#include <platform_stdlib.h>

#include <platform/CHIPDeviceLayer.h>
#include <app/server/Server.h>
#include <platform/Ameba/ConfigurationManagerImpl.h>

bool matter_server_is_commissioned()
{
    return (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0);
}

void matter_factory_reset()
{
    chip::Server::GetInstance().ScheduleFactoryReset();
}
