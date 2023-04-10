#include <platform_stdlib.h>
#include "matter_api.h"

#include <platform/CHIPDeviceLayer.h>
#include <app/server/Server.h>

bool matter_server_is_commissioned()
{
    return (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0);
}

void matter_get_fabric_indexes(uint16_t *pFabricIndexes)
{
    size_t i = 0;
    for (auto it = chip::Server::GetInstance().GetFabricTable().begin(); 
        it != chip::Server::GetInstance().GetFabricTable().end(); ++it)
    {
            ChipLogError(DeviceLayer, "Fabric Index = %d", it->GetFabricIndex());
            pFabricIndexes[i] = it->GetFabricIndex();
            i++;
    }
}
