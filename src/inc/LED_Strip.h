#ifndef LED_STRIP_H
#define LED_STRIP_H

#include <FastLED.h>
#include <Filter.h>
//#include <inc/CRGB_d.h>

class LED_Strip
{
protected:
  int m_num_leds = 0;
  int m_led_mode;
  bool m_on = true;
  int m_brightness;
  int m_brightness_target;
  CRGB *m_leds;
  CRGB *m_leds_raw; //unscaled version
  CRGB m_color_correction;
  CRGB m_color_target;
  Filter<double> m_filter_bri;
  Filter<double> m_filter_color_r;
  Filter<double> m_filter_color_g;
  Filter<double> m_filter_color_b;

  uint8_t ledLinBrightness(uint8_t x)
  {

    uint16_t ret = x / 4; // 0 <= x < 256/4

    if (x >= 64 && x < 128)
    { // 256/4 <= x < 512/4
      ret = x / 2 - 16;
    }
    else if (x >= 128 && x < 192)
    { // 512/4 <= x < 768/4
      ret = x - 80;
    }
    else if (x >= 192 && x < 256)
    { // 768/4 <= x < 1024/4
      ret = 2 * x - 272;
    }
    return ret;
  }

  uint16_t ledLinBrightness_10bit(uint16_t x)
  {

    uint16_t ret = x / 4; // 0 <= x < 256

    if (x >= 256 && x < 512)
    { // 256 <= x < 512
      ret = x / 2 - 64;
    }
    else if (x >= 512 && x < 768)
    { // 512 <= x < 768
      ret = x - 320;
    }
    else if (x >= 768 && x < 1024)
    { // 768 <= x < 1024
      ret = 2 * x - 1088;
    }
    return ret;
  }

  CRGB scaledColor(CRGB c)
  {
    c.r = ledLinBrightness(c.r); //adjust for logarithmic sensation of eye
    c.g = ledLinBrightness(c.g);
    c.b = ledLinBrightness(c.b);

    c.nscale8(m_brightness);       // brightness adjustment 0 == OFF
    c.nscale8(m_color_correction); // correct for led specific color

    return c;
  }

public:
  enum
  {
    MODE_MANY,
    MODE_SINGLE
  };

  LED_Strip(const int p_nleds)
  {
    m_num_leds = max(1, p_nleds);
    m_leds = new CRGB[m_num_leds];
    m_leds_raw = new CRGB[m_num_leds];
    m_led_mode = m_num_leds == 1 ? MODE_SINGLE : MODE_MANY;
    m_color_target = 0;
    m_brightness_target = 0;
    m_color_correction = 0xFFFFFF;
  }

  ~LED_Strip()
  {
    delete[] m_leds;
    delete[] m_leds_raw;
  }

  virtual LED_Strip &init(const CRGB &init_color, const double init_bri, const int transition_time)
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

  virtual LED_Strip &fadeall(const uint8_t amount = 253)
  {
    for (int i = 0; i < m_num_leds; i++)
    {
      m_leds_raw[i].nscale8(amount);
    }
    return *this;
  }

  virtual inline LED_Strip &setColorCorrection(const CRGB &color_correction)
  {
    m_color_correction = color_correction;
    return *this;
  }

  virtual inline LED_Strip &setColorCorrection(const LEDColorCorrection &color_correction)
  {
    m_color_correction = color_correction;
    return *this;
  }

  virtual inline LED_Strip &setBrightness(const uint8_t b)
  {
    m_brightness_target = b;
    return *this;
  }

  virtual inline LED_Strip &setPower(const bool s)
  {
    m_on = s;
    return *this;
  }

  virtual inline bool getPower() { return m_on; }

  virtual LED_Strip &setColor(const CRGB &c)
  {
    m_led_mode = MODE_SINGLE; //set to single mode so all leds are used as one
    m_color_target = c;       //assign color to target
    return *this;
  }

  virtual CRGB &getSingleColor(const int i)
  {
    if (i < 0 || i >= m_num_leds)
      return m_leds[0]; //return first element if out of index
    return m_leds[i];   //return reference to element at index
  }

  virtual LED_Strip &setSingleColor(const CRGB &color, const int i)
  {
    if (i > m_num_leds || i < 0)
      return *this;         //abort if out of index
    m_led_mode = MODE_MANY; //set mode to many to allow individual adressing of leds
    m_leds_raw[i] = color;  //assign correct
    return *this;
  }

  virtual inline LED_Strip &operator<<(const CRGB c)
  {
    return setColor(c);
  }

  virtual inline LED_Strip &operator/=(const uint8_t b)
  {
    return setBrightness(b);
  }

  virtual inline LED_Strip &setMode(uint8_t mode)
  {
    this->m_led_mode = mode;
    return *this;
  }

  virtual CRGB &operator[](const int i)
  {
    return getSingleColor(i);
  }

  virtual LED_Strip &update()
  {

    //update brightness with smooth transition
    int bri_target = m_brightness_target;
    if (!m_on)
    {
      bri_target = 0;
    }
    m_brightness = m_filter_bri.updateLinear(bri_target).getLinear();

    if (m_led_mode == MODE_SINGLE)
    { //calculate smooth color transition and apply to all leds

      m_filter_color_r.updateLinear(m_color_target.r); //update each color seperately
      m_filter_color_g.updateLinear(m_color_target.g);
      m_filter_color_b.updateLinear(m_color_target.b);

      CRGB c = CRGB(m_filter_color_r.getLinear(), m_filter_color_g.getLinear(), m_filter_color_b.getLinear());

      for (uint16_t i = 0; i < m_num_leds; i++)
      {
        m_leds[i] = scaledColor(c); //scaled colors to output
        m_leds_raw[i] = c;          //raw color to raw leds
      }
    }
    else if (m_led_mode == MODE_MANY)
    { //copy from raw data
      for (uint16_t i = 0; i < m_num_leds; i++)
      {
        m_leds[i] = scaledColor(m_leds_raw[i]); //scale color to right brightness
      }
    }

    return *this;
  }
};

#endif //LED_STRIP_H
