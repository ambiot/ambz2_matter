#include <platform_stdlib.h>
#include "matter_api.h"

#include <platform/CHIPDeviceLayer.h>
#include <app/server/Server.h>

bool matter_server_is_commissioned()
{
    return (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0);
}

void matter_get_fabric_indices(uint16_t *pFabricIndices)
{
    size_t i = 0;
    for (auto it = chip::Server::GetInstance().GetFabricTable().begin(); 
        it != chip::Server::GetInstance().GetFabricTable().end(); ++it)
    {
            ChipLogError(DeviceLayer, "Fabric Index = %d", it->GetFabricIndex());
            pFabricIndices[i] = it->GetFabricIndex();
            i++;
    }
}
