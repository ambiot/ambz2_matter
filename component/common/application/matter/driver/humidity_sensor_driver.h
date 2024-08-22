#pragma once

#include <platform_stdlib.h>
#include <pwmout_api.h>
#include <app/util/attribute-table.h>

using namespace ::chip;
using namespace ::chip::app;

class MatterHumiditySensor
{
public:
    void SetEp(EndpointId ep);
    EndpointId GetEp(void);

    void Init(PinName pin);
    void deInit(void);

    uint16_t readHumidity(void);

    // Attribute (0x0000) MeasuredValue
    void setMeasuredValue(uint16_t newHum);
    uint16_t getMeasuredValue(void);

    // Attribute (0x0001) MinMeasuredValue
    void setMinMeasuredValue(uint16_t newHum);
    uint16_t getMinMeasuredValue(void);

    // Attribute (0x0002) MaxMeasuredValue
    void setMaxMeasuredValue(uint16_t newHum);
    uint16_t getMaxMeasuredValue(void);

private:
    EndpointId mEp;
    uint16_t measuredValue;
    uint16_t minMeasuredValue;
    uint16_t maxMeasuredValue;
};
