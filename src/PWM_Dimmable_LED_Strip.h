#pragma once

#include <LED_Strip.h>

class PWM_Dimmable_LED_Strip : public LED_Strip
{
  private:
    uint8_t m_pin;

  public:
    PWM_Dimmable_LED_Strip(uint8_t p_pin)
        : LED_Strip(1), m_pin(p_pin)
    {
        analogWriteFreq(500);
        analogWriteRange(255);
    }

    // implement update method
    virtual void update() override
    {
        LED_Strip::updateLeds();

        uint8_t bri = getBrightness();
        bri = ledLinBrightness(bri);
        analogWrite(m_pin, bri);
    }
};