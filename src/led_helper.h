#pragma once

#include <Arduino.h>

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

CRGB scaledColor(CRGB c, uint8_t brightness, const CRGB &correction)
{
  c.r = ledLinBrightness(c.r); //adjust for logarithmic sensation of eye
  c.g = ledLinBrightness(c.g);
  c.b = ledLinBrightness(c.b);

  c.nscale8(brightness); // brightness adjustment 0 == OFF
  c.nscale8(correction); // correct for led specific color

  return c;
}
