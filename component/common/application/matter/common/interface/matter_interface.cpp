#include "matter_interface.h"
#include <platform_stdlib.h>

#include <platform/CHIPDeviceLayer.h>
#include <app/server/Server.h>

bool matter_server_is_commissioned()
{
    return (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0);
}
