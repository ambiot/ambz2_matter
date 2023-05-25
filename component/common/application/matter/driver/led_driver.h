#pragma once

#include <platform_stdlib.h>
#include "pwmout_api.h"
#include "pwmout_ex_api.h"

class MatterLED
{
public:
    void Init(PinName pin);
    void Init(PinName redpin, PinName greenpin, PinName bluepin);
    void Init(PinName redpin, PinName greenpin, PinName bluepin, PinName cwhitepin, PinName wwhitepin);
    void deInit(void);
    uint8_t GetLevel(void);
    bool IsTurnedOn(void);
    void Set(bool state);
    void Toggle(void);
    void SetBrightness(uint8_t brightness);
    void SetColor(uint8_t Hue, uint8_t Saturation);
    void SetColorTemp(uint16_t colortemp);
    void HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness, uint8_t & red, uint8_t & green, uint8_t & blue);
    void simpleRGB2RGBW(uint8_t & red, uint8_t & green, uint8_t & blue, uint8_t & cwhite, uint8_t & wwhite);
    uint8_t mBrightness;
    uint16_t mHue;       // mHue [0, 360]
    uint8_t mSaturation; // mSaturation [0, 100]
    uint16_t mColorTemp;

private:
    pwmout_t *mPwm_obj = NULL;
    pwmout_t *mPwm_red = NULL;
    pwmout_t *mPwm_green = NULL;
    pwmout_t *mPwm_blue = NULL;
    pwmout_t *mPwm_cwhite = NULL;
    pwmout_t *mPwm_wwhite = NULL;
    bool mRgb = false;
    bool mRgbw = false;
    bool mState;
    void DoSet();
    uint16_t WhitePercentage[11][3] = 
    {
        /*CT--coolwhite%--warmwhite%*/
        {2708, 0, 100},
        {2891, 10, 90},
        {3110, 20, 80},
        {3364, 30, 70},
        {3656, 40, 60},
        {3992, 50, 50},
        {4376, 60, 40},
        {4809, 70, 30},
        {5304, 80, 20},
        {5853, 90, 10},
        {6471, 100, 0}
    };

};
// #define PWM_LED                     PA_23
// #define GREEN_LED_GPIO_NUM          PA_19
// #define RED_LED_GPIO_NUM            PA_18
// #define BLUE_LED_GPIO_NUM           PA_20
// #define COOL_WHITE_LED_GPIO_NUM     PA_4
// #define WARM_WHITE_LED_GPIO_NUM     PA_17
