#ifndef LED_STRIP_H
#define LED_STRIP_H

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#include <Filter.h>
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
  int m_num_leds = 0;
  MODE m_led_mode = MODE::SINGLE;
  bool m_on = true;
  int m_brightness = 0;
  int m_brightness_target = 0;
  CRGB *m_leds;
  CRGB *m_leds_raw; //unscaled version
  CRGB m_color_correction = 0xFFFFFF;
  CRGB m_color_target = 0;
  // filters for smooth transition
  Filter<> m_filter_bri;
  Filter<> m_filter_color_r;
  Filter<> m_filter_color_g;
  Filter<> m_filter_color_b;

  /**
   * internal method to update led calculation
   * this function must be called at the beginning of every LED_Strip update implementation
  */
  LED_Strip &updateLeds()
  {
    //update brightness with smooth transition
    if (m_on)
    {
      // strip is ON
      m_filter_bri.updateLinear(m_brightness_target);
    }
    else
    {
      // strip is OFF
      m_filter_bri.updateLinear(0);
    }
    m_brightness = m_filter_bri.getLinear();

    // single mode -> the entire strip acts as one led
    if (m_led_mode == MODE::SINGLE)
    {
      // calculate smooth color transition and apply to all leds
      // update each color separately
      m_filter_color_r.updateLinear(m_color_target.r);
      m_filter_color_g.updateLinear(m_color_target.g);
      m_filter_color_b.updateLinear(m_color_target.b);

      // create color object from animated r g b values
      CRGB c = CRGB(m_filter_color_r.getLinear(), m_filter_color_g.getLinear(), m_filter_color_b.getLinear());

      for (uint16_t i = 0; i < m_num_leds; i++)
      {
        // scaled colors to output
        m_leds[i] = scaledColor(c, m_brightness, m_color_correction);
        // raw color to raw leds
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
        m_leds[i] = scaledColor(m_leds_raw[i], m_brightness, m_color_correction);
      }
    }

    return *this;
  }

public:
  LED_Strip(const int p_nleds)
  {
    m_num_leds = max(1, p_nleds);
    m_led_mode = m_num_leds == 1 ? MODE::SINGLE : MODE::MANY;

    m_leds = new CRGB[m_num_leds];
    m_leds_raw = new CRGB[m_num_leds];
  }

  ~LED_Strip()
  {
    delete[] m_leds;
    delete[] m_leds_raw;
  }

  LED_Strip &init(const CRGB &init_color, const double init_bri, const int transition_time)
  {

    m_brightness_target = init_bri;
    m_filter_bri.initLinear(init_bri, transition_time);

    m_filter_color_r.initLinear(init_color.r, transition_time);
    m_filter_color_g.initLinear(init_color.g, transition_time);
    m_filter_color_b.initLinear(init_color.b, transition_time);

    for (uint16_t i = 0; i < m_num_leds; i++)
    {
      m_leds[i] = init_color;
      m_leds_raw[i] = init_color;
    }
    return *this;
  }

  LED_Strip &fadeall(const uint8_t amount = 253)
  {
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

  inline LED_Strip &setBrightness(const uint8_t b)
  {
    m_brightness_target = b;
    return *this;
  }

  inline uint8_t getBrightness()
  {
    return m_brightness;
  }

  inline LED_Strip &setPower(const bool s)
  {
    m_on = s;
    return *this;
  }

  inline bool getPower()
  {
    return m_on;
  }

  LED_Strip &setColor(const CRGB &c)
  {
    m_led_mode = MODE::SINGLE; //set to single mode so all leds are used as one
    m_color_target = c;        //assign color to target
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
