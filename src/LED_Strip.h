#ifndef LED_STRIP_H
#define LED_STRIP_H

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#include <FilterLinear.h>
#include <CRGB_d.h>

#include "led_helper.h"

class LED_Strip
{

public:
  enum MODE
  {
    MANY,
    SINGLE
  };

protected:
  MODE m_led_mode = MODE::SINGLE; // treat leds as one single color or individually

  uint16_t m_num_leds = 0; // keep track of number of leds

  bool m_power = 0;                // only necessary for setting power
  uint8_t m_brightness_target = 0; // save state of brightness

  uint16_t m_transition_time = 0; // for determining if leds should be updated
  unsigned long m_last_update = 0;

  CRGB m_color_correction = 0xFFFFFF; // apply color correction to leds if not every color has equal brightness

  CRGB *m_leds;     // store led color in array mostly necessary for fastled
  CRGB *m_leds_raw; //unscaled version

  FilterLinear m_filter_bri; // filters for smooth transition
  FilterLinear m_filter_color_r;
  FilterLinear m_filter_color_g;
  FilterLinear m_filter_color_b;

  /**
   * internal method to update led calculation
   * this function must be called at the beginning of every LED_Strip update implementation
  */
  LED_Strip &updateLeds()
  {
    // update brightness filter
    m_filter_bri.setTarget(m_power ? m_brightness_target : 0);
    m_filter_bri.update();
    // calculate smooth color transition and apply to all leds
    // update each color separately
    m_filter_color_r.update();
    m_filter_color_g.update();
    m_filter_color_b.update();

    // single mode -> the entire strip acts as one led
    if (m_led_mode == MODE::SINGLE)
    {
      // create color object from animated r g b values
      CRGB c = CRGB(m_filter_color_r.getValue(), m_filter_color_g.getValue(), m_filter_color_b.getValue());

      // calculate adjusted version of color so perceived brightness is linear
      CRGB scaled_color = scaledColor(c, m_filter_bri.getValue(), m_color_correction);

      for (uint16_t i = 0; i < m_num_leds; i++)
      {
        // scaled color to output
        m_leds[i] = scaled_color;
        // store raw color in raw_leds array to allow for modification in individual mode
        m_leds_raw[i] = c;
      }
    }
    // multi mode -> each led is separately addressable
    else if (m_led_mode == MODE::MANY)
    {
      // copy from raw data
      for (uint16_t i = 0; i < m_num_leds; i++)
      {
        // scale color to right brightness
        m_leds[i] = scaledColor(m_leds_raw[i], m_filter_bri.getValue(), m_color_correction);
      }
    }

    return *this;
  }

  /**
   * determine if leds should be updated
   * @returns true if update necessary else false
  */
  bool isUpdateNecessary()
  {
    if (millis() > m_last_update + m_transition_time)
      return false;
    return true;
  }

public:
  LED_Strip(const unsigned int p_nleds)
  {
    m_num_leds = max(1u, p_nleds);
    m_led_mode = m_num_leds == 1 ? MODE::SINGLE : MODE::MANY;

    m_leds = new CRGB[m_num_leds];
    m_leds_raw = new CRGB[m_num_leds];
  }

  ~LED_Strip()
  {
    delete[] m_leds;
    delete[] m_leds_raw;
  }

  LED_Strip &init(const CRGB &init_color, const uint8_t init_bri, const uint16_t transition_time)
  {
    m_transition_time = transition_time;

    m_power = init_bri > 0 ? 1 : 0;
    m_filter_bri.init(init_bri, transition_time);

    m_filter_color_r.init(init_color.r, transition_time);
    m_filter_color_g.init(init_color.g, transition_time);
    m_filter_color_b.init(init_color.b, transition_time);

    for (uint16_t i = 0; i < m_num_leds; i++)
    {
      m_leds[i] = init_color;
      m_leds_raw[i] = init_color;
    }
    return *this;
  }

  LED_Strip &forceUpdate()
  {
    m_last_update = millis();
    return *this;
  }

  LED_Strip &fadeall(const uint8_t amount = 253)
  {
    m_last_update = millis();
    for (int i = 0; i < m_num_leds; i++)
    {
      m_leds_raw[i].nscale8(amount);
    }
    return *this;
  }

  inline LED_Strip &setColorCorrection(const CRGB &color_correction)
  {
    m_color_correction = color_correction;
    return *this;
  }

  inline uint8_t getNumLeds()
  {
    return m_num_leds;
  }

  inline LED_Strip &setBrightness(const uint8_t b)
  {
    m_last_update = millis();

    m_brightness_target = b;

    return *this;
  }

  inline uint8_t getBrightness()
  {
    return m_filter_bri.getValue();
  }

  inline LED_Strip &setPower(const bool s)
  {
    m_last_update = millis();

    m_power = s;

    return *this;
  }

  inline bool getPower()
  {
    return m_power;
  }

  LED_Strip &setColor(const CRGB &c)
  {
    m_last_update = millis();

    m_led_mode = MODE::SINGLE; //set to single mode so all leds are used as one

    m_filter_color_r.setTarget(c.r);
    m_filter_color_g.setTarget(c.g);
    m_filter_color_b.setTarget(c.b);
    return *this;
  }

  inline CRGB &getColor()
  {
    return m_leds[0];
  }

  inline LED_Strip &setMode(MODE mode)
  {
    this->m_led_mode = mode;
    return *this;
  }

  inline MODE getMode()
  {
    return m_led_mode;
  }

  inline LED_Strip &operator<<(const CRGB c)
  {
    return setColor(c);
  }

  inline LED_Strip &operator/=(const uint8_t b)
  {
    return setBrightness(b);
  }

  // virtual interface method for updating
  virtual void update() = 0;
};

#endif //LED_STRIP_H
