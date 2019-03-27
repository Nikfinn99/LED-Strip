#ifndef ADRESSABLE_LED_STRIP_H
#define ADRESSABLE_LED_STRIP_H

#include <LED_Strip.h>

class Adressable_LED_Strip : public LED_Strip
{
protected:
  uint8_t m_LastHue = 0;
  int m_PointPosition = 0;
  bool m_PointDirection = true;

public:
  Adressable_LED_Strip() : LED_Strip(1) {}
  Adressable_LED_Strip(const int p_nleds) : LED_Strip(p_nleds) {}

  Adressable_LED_Strip &setSingleColor(const CRGB &color, const int i)
  {
    if (i > m_num_leds || i < 0)
      return *this;          //abort if out of index
    m_led_mode = MODE::MANY; //set mode to many to allow individual adressing of leds
    m_leds_raw[i] = color;   //assign correct
    return *this;
  }

  CRGB &getSingleColor(const int i)
  {
    if (i < 0 || i >= m_num_leds)
      return m_leds[0]; //return first element if out of index
    return m_leds[i];   //return reference to element at index
  }

  inline CRGB &operator[](const int i)
  {
    return getSingleColor(i);
  }

  Adressable_LED_Strip &sparkle()
  {
    setMode(MANY);

    int id = 0;
    fadeall(220);
    for (int i = 0; i < 1; i++)
    {
      id = map(random8(), 0, 255, 0, m_num_leds - 1);
      setSingleColor(CRGB(255, 255, random8()), id);
    }
    return *this;
  }

  Adressable_LED_Strip &sectionColor(const int sec_size)
  {
    if (sec_size <= 0)
      return *this;

    setMode(MANY);

    for (int i = 0; i < m_num_leds; i++)
    {
      switch ((i / sec_size) % 3)
      {
      case 0:
        setSingleColor(CRGB::Red, i);
        break;
      case 1:
        setSingleColor(CRGB::Green, i);
        break;
      case 2:
        setSingleColor(CRGB::Blue, i);
        break;
      }
    }
    return *this;
  }

  Adressable_LED_Strip &spectrumHue()
  {
    setMode(MANY);

    for (int i = m_num_leds - 1; i >= 1; i--)
    {
      m_leds_raw[i] = m_leds_raw[i - 1];
    }
    setSingleColor(CHSV(m_LastHue++, 255, 255), 0);
    return *this;
  }

  Adressable_LED_Strip &movingPoint(const CRGB &c)
  {
    setMode(MANY);

    setSingleColor(c, m_PointPosition);
    fadeall(250);
    if (m_PointDirection)
    {
      m_PointPosition++;
      if (m_PointPosition == m_num_leds)
      {
        m_PointDirection = false;
        m_PointPosition = m_num_leds - 1;
      }
    }
    else
    {
      m_PointPosition--;
      if (m_PointPosition == -1)
      {
        m_PointDirection = true;
        m_PointPosition = 0;
      }
    }
    return *this;
  }

  virtual Adressable_LED_Strip &movingHue()
  {
    movingPoint(CHSV(m_LastHue++, 255, 255));
    return *this;
  }
};

#endif //ADRESSABLE_LED_STRIP_H
