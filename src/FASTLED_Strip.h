#ifndef FASTLED_STRIP_H
#define FASTLED_STRIP_H

#include <Adressable_LED_Strip.h>

//interface class to add fastled support
template <template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB> class CHIPSET, uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class FASTLED_Strip : public Adressable_LED_Strip
{
protected:
  CLEDController *chipset;

public:
  FASTLED_Strip(const int p_nleds) : Adressable_LED_Strip(p_nleds)
  { //setup CLEDController
    chipset = &(FastLED.addLeds<CHIPSET, DATA_PIN, RGB_ORDER>(m_leds, m_num_leds));
    FastLED.setDither(0);
  }

  virtual void update() override
  { //update and show leds from this controller
    LED_Strip::updateLeds();

    // find out if an update is necessary to prevent unnecessary writes using fastled and stop crashing
    if (isUpdateNecessary())
    {
      chipset->showLeds();
    }
  }
};

#endif //FASTLED_STRIP_H
