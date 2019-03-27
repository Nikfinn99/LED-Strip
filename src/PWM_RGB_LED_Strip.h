#ifndef PWM_LED_STRIP_H
#define PWM_LED_STRIP_H

#include <LED_Strip.h>

class PWM_RGB_LED_Strip : public LED_Strip
{
protected:
  uint8_t m_pwm_r, m_pwm_g, m_pwm_b;

public:
  PWM_RGB_LED_Strip(const uint8_t pwm_R, const uint8_t pwm_G, const uint8_t pwm_B)
      : LED_Strip(1), m_pwm_r(pwm_R), m_pwm_g(pwm_G), m_pwm_b(pwm_B)
  {
    analogWriteFreq(500);
    analogWriteRange(255);
  }

  // implement update method
  virtual void update() override
  {
    LED_Strip::updateLeds();

    CRGB &col = getColor();
    analogWrite(m_pwm_r, col.r);
    analogWrite(m_pwm_g, col.g);
    analogWrite(m_pwm_b, col.b);
  }
};

#endif //PWM_LED_STRIP_H
