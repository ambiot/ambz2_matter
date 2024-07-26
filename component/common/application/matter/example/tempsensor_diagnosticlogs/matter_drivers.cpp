#include "matter_drivers.h"
#include "matter_interaction.h"
#include "temperature_sensor_driver.h"

#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <protocols/interaction_model/StatusCode.h>

#if defined(CONFIG_EXAMPLE_MATTER_TEMPSENSOR_DIAGNOSTICLOGS) && CONFIG_EXAMPLE_MATTER_TEMPSENSOR_DIAGNOSTICLOGS
#if defined(CONFIG_AMEBA_ENABLE_DIAGNOSTIC_LOGS) && (CONFIG_AMEBA_ENABLE_DIAGNOSTIC_LOGS == 1)
#include "ameba_diagnosticlogs_provider_delegate_impl.h"

#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>
using namespace chip::app::Clusters::DiagnosticLogs;

/* Attach the log provider delegate to the server via callback */
void emberAfDiagnosticLogsClusterInitCallback(chip::EndpointId endpoint) {
    auto & logProvider = AmebaDiagnosticLogsProvider::GetInstance();
    DiagnosticLogsServer::Instance().SetDiagnosticLogsProviderDelegate(endpoint, &logProvider);
}
#endif
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureMeasurement;
using chip::Protocols::InteractionModel::Status;

template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;
using Status            = Protocols::InteractionModel::Status;

MatterTemperatureSensor TemperatureSensor;

CHIP_ERROR matter_driver_tempsensor_init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_tempsensor_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Status status;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    TemperatureSensor.setMeasuredTemperature(37);   // currently no-op
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

exit:
    if (err == CHIP_ERROR_INTERNAL)
    {
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    return err;
}

CHIP_ERROR matter_driver_tempsensor_start()
{
    TemperatureSensor.startPollingTask();
    return CHIP_NO_ERROR;
}

void matter_driver_set_opstate_callback(uint32_t id)
{
    return;   // NO OP
}

void matter_driver_uplink_update_handler(AppEvent *aEvent)
{
    return;   // no OP
}

void matter_driver_downlink_update_handler(AppEvent *event)
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    switch (event->Type)
    {
    default:
        break;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}