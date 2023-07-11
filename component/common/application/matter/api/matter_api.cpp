#include <platform/CHIPDeviceLayer.h>
#include <app/server/Server.h>

#include <platform_stdlib.h>
#include "matter_api.h"
// #include <app/server/CommissioningWindowManager.h>

bool matter_server_is_commissioned()
{
    return (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0);
}

void matter_get_fabric_indexes(uint16_t *pFabricIndexes, uint8_t bufSize)
{
    size_t i = 0;
    for (auto it = chip::Server::GetInstance().GetFabricTable().begin(); 
        it != chip::Server::GetInstance().GetFabricTable().end(); ++it)
    {
        if (bufSize < i)
        {
            // out of buffer space
            ChipLogError(DeviceLayer, "Returning... buffer too small");
            return;
        }
        ChipLogProgress(DeviceLayer, "Fabric Index = %d", it->GetFabricIndex());
        pFabricIndexes[i] = it->GetFabricIndex();
        i++;
    }
}

void matter_open_basic_commissioning_window()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
