#pragma once

#include <credentials/FabricTable.h>
#include <app/server/Server.h>

using namespace ::chip;
using namespace ::chip::app;

class AppFabricTableDelegate : public FabricTable::Delegate
{
public:
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        ChipLogProgress(DeviceLayer, "Ameba Observer: Fabric has been Removed");

        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            //Customer code
        }
    }
};

AppFabricTableDelegate AmebaObserver;

