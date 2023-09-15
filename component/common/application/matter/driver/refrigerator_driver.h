#pragma once

#include <platform_stdlib.h>
#include "pwmout_api.h"

class MatterRefrigerator
{
public:
    void Init(PinName pin);
    void deInit(void);
    void Do(void);

private:
    pwmout_t *mPwm_obj = NULL;
};
