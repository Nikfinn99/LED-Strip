#ifndef PWM_LED_STRIP_H
#define PWM_LED_STRIP_H

#include <inc/LED_Strip.h>

class PWM_LED_Strip : public LED_Strip
{
protected:
  uint8_t m_pwm_r, m_pwm_g, m_pwm_b;

public:
  PWM_LED_Strip(const uint8_t pwm_R, const uint8_t pwm_G, const uint8_t pwm_B) : LED_Strip(1)
  {
    analogWriteFreq(500);
    analogWriteRange(255);
    m_pwm_r = pwm_R;
    m_pwm_g = pwm_G;
    m_pwm_b = pwm_B;
  }

  virtual PWM_LED_Strip &update()
  {
    LED_Strip::update();
    CRGB &col = getSingleColor(0);
    analogWrite(m_pwm_r, col.r);
    analogWrite(m_pwm_g, col.g);
    analogWrite(m_pwm_b, col.b);
    return *this;
  }
};

#endif //PWM_LED_STRIP_H
