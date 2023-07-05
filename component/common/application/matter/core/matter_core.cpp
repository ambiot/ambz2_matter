#include <stdlib.h>
#include <stdint.h>

#include "matter_core.h"
#include "matter_ota_initializer.h"
#include <DeviceInfoProviderImpl.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/DeferredAttributePersistenceProvider.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/dnssd/Advertiser.h>
#include <platform/Ameba/AmebaUtils.h>

#include <platform/Ameba/AmebaConfig.h>
#include <platform/Ameba/FactoryDataProvider.h>
#include <platform/Ameba/NetworkCommissioningDriver.h>

#include <route_hook/ameba_route_hook.h>

#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>

#if CONFIG_ENABLE_CHIP_SHELL
#include <shell/launch_shell.h>
#endif

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::DeviceLayer;


// Define a custom attribute persister which makes actual write of the selected attribute values
// to the non-volatile storage only when it has remained constant for 5 seconds. This is to reduce
// the flash wearout when the attribute changes frequently as a result of MoveToLevel command.
// DeferredAttribute object describes a deferred attribute, but also holds a buffer with a value to
// be written, so it must live so long as the DeferredAttributePersistenceProvider object.

DeferredAttribute gDeferredAttributeArray[] = {
    DeferredAttribute(ConcreteAttributePath(1 /* kLightEndpointId */, Clusters::LevelControl::Id, Clusters::LevelControl::Attributes::CurrentLevel::Id)),
    DeferredAttribute(ConcreteAttributePath(1 /* kLightEndpointId */, Clusters::ColorControl::Id, Clusters::ColorControl::Attributes::CurrentHue::Id)),
    DeferredAttribute(ConcreteAttributePath(1 /* kLightEndpointId */, Clusters::ColorControl::Id, Clusters::ColorControl::Attributes::CurrentSaturation::Id)),
    DeferredAttribute(ConcreteAttributePath(1 /* kLightEndpointId */, Clusters::ColorControl::Id, Clusters::ColorControl::Attributes::EnhancedCurrentHue::Id)),
    DeferredAttribute(ConcreteAttributePath(1 /* kLightEndpointId */, Clusters::ColorControl::Id, Clusters::ColorControl::Attributes::CurrentX::Id)),
    DeferredAttribute(ConcreteAttributePath(1 /* kLightEndpointId */, Clusters::ColorControl::Id, Clusters::ColorControl::Attributes::CurrentY::Id)),
};
DeferredAttributePersistenceProvider gDeferredAttributePersister(Server::GetInstance().GetDefaultAttributePersister(), Span<DeferredAttribute>(gDeferredAttributeArray, 6), System::Clock::Milliseconds32(5000));

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::AmebaWiFiDriver::GetInstance()));

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
chip::DeviceLayer::FactoryDataProvider mFactoryDataProvider;

void matter_core_device_callback_internal(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kInternetConnectivityChange:
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        static bool isOTAInitialized = false; // use this static variable to replace CheckInit()
#endif
        if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
        {
            ChipLogProgress(DeviceLayer, "IPv4 Server ready...");
            chip::app::DnssdServer::Instance().StartServer();
        }
        else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
        {
            ChipLogProgress(DeviceLayer, "Lost IPv4 connectivity...");
        }
        if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
        {
            ChipLogProgress(DeviceLayer, "IPv6 Server ready...");
            chip::app::DnssdServer::Instance().StartServer();

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
            // Init OTA requestor only when we have gotten IPv6 address
            if (!isOTAInitialized)
            {
                matter_ota_initializer();
                isOTAInitialized = true;
            }
#endif
        }
        else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
        {
            ChipLogProgress(DeviceLayer, "Lost IPv6 connectivity...");
        }
        break;

    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();
        }
        if (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned)
        {
            ChipLogProgress(DeviceLayer, "Initializing route hook...");
            ameba_route_hook_init();
        }
        break;
    case DeviceEventType::kCommissioningComplete:
        ChipLogProgress(DeviceLayer, "Commissioning Complete");
        chip::DeviceLayer::Internal::AmebaUtils::SetCurrentProvisionedNetwork();
        break;
    }
}

void matter_core_init_server(intptr_t context)
{
    xTaskHandle task_to_notify = reinterpret_cast<xTaskHandle>(context);
    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    initParams.InitializeStaticResourcesBeforeServerInit();
    chip::Server::GetInstance().Init(initParams);
    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    // TODO: Use our own DeviceInfoProvider
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);
    SetAttributePersistenceProvider(&gDeferredAttributePersister);

    sWiFiNetworkCommissioningInstance.Init();

    // We only have network commissioning on endpoint 0.
    // TODO: configure the endpoint
    emberAfEndpointEnableDisable(0xFFFE, false);

    if (RTW_SUCCESS != wifi_is_connected_to_ap())
    {
        // QR code will be used with CHIP Tool
        PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
    }

#if CONFIG_ENABLE_CHIP_SHELL
    InitBindingHandler();
#endif

    xTaskNotifyGive(task_to_notify);
}

CHIP_ERROR matter_core_init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err = Platform::MemoryInit();
    SuccessOrExit(err);

    // Initialize the CHIP stack.
    err = PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    err = mFactoryDataProvider.Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error initializing FactoryData!");
        ChipLogError(DeviceLayer, "Check if you have flashed it correctly!");
    }
    SetCommissionableDataProvider(&mFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);

    if (CONFIG_NETWORK_LAYER_BLE)
    {
        ConnectivityMgr().SetBLEAdvertisingEnabled(true);
    }

    // Start a task to run the CHIP Device event loop.
    err = PlatformMgr().StartEventLoopTask();
    SuccessOrExit(err);

    // Register a function to receive events from the CHIP device layer.  Note that calls to
    // this function will happen on the CHIP event loop thread, not the app_main thread.
    PlatformMgr().AddEventHandler(matter_core_device_callback_internal, reinterpret_cast<intptr_t>(NULL));

    // PlatformMgr().ScheduleWork(matter_core_init_server, 0);
    PlatformMgr().ScheduleWork(matter_core_init_server, reinterpret_cast<intptr_t>(xTaskGetCurrentTaskHandle()));
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

exit:
    return err;
}

CHIP_ERROR matter_core_start()
{
    return matter_core_init();
    // matter_core_init_server();
}
