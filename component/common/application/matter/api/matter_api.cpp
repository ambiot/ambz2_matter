#include <app/server/Server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>

#include <platform_stdlib.h>
#include "matter_api.h"

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

bool matter_server_is_commissioned()
{
    return (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0);
}

void matter_get_fabric_indexes(uint16_t *pFabricIndexes, size_t bufSize)
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

CHIP_ERROR matter_get_manual_pairing_code(char *buf, size_t bufSize)
{
    if (bufSize < chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1)
    {
        ChipLogError(DeviceLayer, "Buffer too small for onboarding code!");
        return CHIP_ERROR_INTERNAL;
    }

    char payloadBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan manualPairingCode(payloadBuffer);
    RendezvousInformationFlags rendezvousFlag = chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE);
    PayloadContents payload;
    
    if (GetPayloadContents(payload, rendezvousFlag) != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get onboarding payload contents");
        return CHIP_ERROR_INTERNAL;
    }

    if (GetManualPairingCode(manualPairingCode, payload) != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to generate manual pairing code!");
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(buf, manualPairingCode.data(), manualPairingCode.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_get_qr_code(char *buf, size_t bufSize)
{
    if (bufSize < chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1)
    {
        ChipLogError(DeviceLayer, "Buffer too small for onboarding code!");
        return CHIP_ERROR_INTERNAL;
    }

    char payloadBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan qrCode(payloadBuffer);
    RendezvousInformationFlags rendezvousFlag = chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE);
    PayloadContents payload;
    
    if (GetPayloadContents(payload, rendezvousFlag) != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get onboarding payload contents");
        return CHIP_ERROR_INTERNAL;
    }

    if (GetQRCode(qrCode, payload) != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to generate qr code!");
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(buf, qrCode.data(), qrCode.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_open_basic_commissioning_window()
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    CommissioningWindowManager *mgr = &(chip::Server::GetInstance().GetCommissioningWindowManager());

    if (mgr != NULL)
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        err = mgr->OpenBasicCommissioningWindow();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }

    return err;
}

CHIP_ERROR matter_get_certificate_declaration(MutableByteSpan & outBuffer)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceAttestationCredentialsProvider *dacProvider = chip::Credentials::GetDeviceAttestationCredentialsProvider();

    if (dacProvider != NULL)
    {
        err = dacProvider->GetCertificationDeclaration(outBuffer);
    }

    return err;
}

CHIP_ERROR matter_get_dac_cert(MutableByteSpan & outBuffer)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceAttestationCredentialsProvider *dacProvider = chip::Credentials::GetDeviceAttestationCredentialsProvider();

    if (dacProvider != NULL)
    {
        err = dacProvider->GetDeviceAttestationCert(outBuffer);
    }

    return err;
}

CHIP_ERROR matter_get_pai_cert(MutableByteSpan & outBuffer)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceAttestationCredentialsProvider *dacProvider = chip::Credentials::GetDeviceAttestationCredentialsProvider();

    if (dacProvider != NULL)
    {
        err = dacProvider->GetProductAttestationIntermediateCert(outBuffer);
    }

    return err;
}

CHIP_ERROR matter_get_firmware_information(MutableByteSpan & outBuffer)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceAttestationCredentialsProvider *dacProvider = chip::Credentials::GetDeviceAttestationCredentialsProvider();

    if (dacProvider != NULL)
    {
        err = dacProvider->GetFirmwareInformation(outBuffer);
    }

    return err;
}

CHIP_ERROR matter_get_setup_discriminator(uint16_t & discriminator)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    CommissionableDataProvider *cdProvider = chip::DeviceLayer::GetCommissionableDataProvider();

    if (cdProvider != NULL)
    {
        err = cdProvider->GetSetupDiscriminator(discriminator);
    }

    return err;
}

CHIP_ERROR matter_get_spake2p_iteration_count(uint32_t & iterationCount)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    CommissionableDataProvider *cdProvider = chip::DeviceLayer::GetCommissionableDataProvider();

    if (cdProvider != NULL)
    {
        err = cdProvider->GetSpake2pIterationCount(iterationCount);
    }

    return err;
}

CHIP_ERROR matter_get_spake2p_salt(MutableByteSpan & saltBuf)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    CommissionableDataProvider *cdProvider = chip::DeviceLayer::GetCommissionableDataProvider();

    if (cdProvider != NULL)
    {
        err = cdProvider->GetSpake2pSalt(saltBuf);
    }

    return err;
}

CHIP_ERROR matter_get_spake2p_verifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    CommissionableDataProvider *cdProvider = chip::DeviceLayer::GetCommissionableDataProvider();

    if (cdProvider != NULL)
    {
        err = cdProvider->GetSpake2pVerifier(verifierBuf, verifierLen);
    }

    return err;
}

CHIP_ERROR matter_get_setup_passcode(uint32_t & passcode)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    CommissionableDataProvider *cdProvider = chip::DeviceLayer::GetCommissionableDataProvider();

    if (cdProvider != NULL)
    {
        err = cdProvider->GetSetupPasscode(passcode);
    }

    return err;
}

CHIP_ERROR matter_get_vendor_name(char *buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetVendorName(buf, bufSize);
    }

    return err;
}

CHIP_ERROR matter_get_vendor_id(uint16_t & vendorId)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetVendorId(vendorId);
    }

    return err;
}

CHIP_ERROR matter_get_product_name(char *buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetProductName(buf, bufSize);
    }

    return err;
}

CHIP_ERROR matter_get_product_id(uint16_t & productId)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetProductId(productId);
    }

    return err;
}

CHIP_ERROR matter_get_part_number(char *buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetPartNumber(buf, bufSize);
    }

    return err;
}

CHIP_ERROR matter_get_product_url(char *buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetProductURL(buf, bufSize);
    }

    return err;
}

CHIP_ERROR matter_get_product_label(char *buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetProductLabel(buf, bufSize);
    }

    return err;
}

CHIP_ERROR matter_get_serial_number(char *buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetSerialNumber(buf, bufSize);
    }

    return err;
}

CHIP_ERROR matter_get_manufacturing_date(uint16_t & year, uint8_t & month, uint8_t & day)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetManufacturingDate(year, month, day);
    }

    return err;
}

CHIP_ERROR matter_get_hardware_version(uint16_t & hardwareVersion)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetHardwareVersion(hardwareVersion);
    }

    return err;
}

CHIP_ERROR matter_get_hardware_version_string(char *buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    DeviceInstanceInfoProvider *diiProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (diiProvider != NULL)
    {
        err = diiProvider->GetHardwareVersionString(buf, bufSize);
    }

    return err;
}

CHIP_ERROR matter_get_software_version(uint32_t & softwareVersion)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    err = chip::DeviceLayer::ConfigurationMgr().GetSoftwareVersion(softwareVersion);
    return err;
}

CHIP_ERROR matter_get_software_version_string(char *buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    err = chip::DeviceLayer::ConfigurationMgr().GetSoftwareVersionString(buf, bufSize);
    return err;
}
