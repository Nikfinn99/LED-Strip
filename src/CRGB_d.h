#ifndef CRGB_D_H
#define CRGB_D_H

#include <FastLED.h>
#include <Printable.h>

class CRGB_d : public Printable
{
public:
  union {
    struct {
      union {
        double r;
        double red;
      };
      union {
        double g;
        double green;
      };
      union {
        double b;
        double blue;
      };
    };
    double raw[3];
  };

  size_t printTo(Print& p) const{//allow printing to Serial
    size_t s = 0;
    s += p.print("r:");
    s += p.print(r);
    s += p.print(" g:");
    s += p.print(g);
    s += p.print(" b:");
    s += p.print(b);
    return s;
  }

  /// Array access operator to index into the crgb object
  inline double& operator[] (uint8_t x) __attribute__((always_inline))
  {
    return raw[x];
  }

  /// Array access operator to index into the crgb object
  inline const double& operator[] (uint8_t x) const __attribute__((always_inline))
  {
    return raw[x];
  }

  ///default constructor -> init with zero
  inline CRGB_d() __attribute__((always_inline))
    : r(0), g(0), b(0)
  {
  }

  /// allow construction from R, G, B
  inline CRGB_d( double ir, double ig, double ib)  __attribute__((always_inline))
    : r(ir), g(ig), b(ib)
  {
  }

  /// allow construction from 32-bit (really 24-bit) bit 0xRRGGBB color code
  inline CRGB_d( uint32_t colorcode)  __attribute__((always_inline))
    : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
  {
  }

  /// allow construction from a LEDColorCorrection enum
  inline CRGB_d( LEDColorCorrection colorcode) __attribute__((always_inline))
    : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
  {

  }

  /// allow construction from a ColorTemperature enum
  inline CRGB_d( ColorTemperature colorcode) __attribute__((always_inline))
    : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
  {

  }

  /// allow copy construction
  inline CRGB_d(const CRGB_d& rhs) __attribute__((always_inline))
  {
    r = rhs.r;
    g = rhs.g;
    b = rhs.b;
  }

  /// allow copy construction
  inline CRGB_d(const CRGB& rhs) __attribute__((always_inline))
  {
    r = rhs.r;
    g = rhs.g;
    b = rhs.b;
  }

  /// allow construction from HSV color
  inline CRGB_d(const CHSV& rhs) __attribute__((always_inline))
  {
    CRGB c;
    hsv2rgb_rainbow( rhs, c);
    *this = c;
  }

  /// allow assignment from one RGB struct to another
  inline CRGB_d& operator= (const CRGB_d& rhs) __attribute__((always_inline))
  {
    r = rhs.r;
    g = rhs.g;
    b = rhs.b;
    fixOverflow8 ();
    return *this;
  }

  /// allow assignment from one RGB struct to another
  inline CRGB_d& operator= (const CRGB& rhs) __attribute__((always_inline))
  {
    r = rhs.r;
    g = rhs.g;
    b = rhs.b;
    fixOverflow8 ();
    return *this;
  }

  /// allow assignment from 32-bit (really 24-bit) 0xRRGGBB color code
  inline CRGB_d& operator= (const uint32_t colorcode) __attribute__((always_inline))
  {
    r = (colorcode >> 16) & 0xFF;
    g = (colorcode >>  8) & 0xFF;
    b = (colorcode >>  0) & 0xFF;
    fixOverflow8 ();
    return *this;
  }

  /// allow assignment from R, G, and B
  inline CRGB_d& setRGB (double nr, double ng, double nb) __attribute__((always_inline))
  {
    r = nr;
    g = ng;
    b = nb;
    fixOverflow8 ();
    return *this;
  }

  /// allow assignment from H, S, and V
  inline CRGB_d& setHSV (uint8_t hue, uint8_t sat, uint8_t val) __attribute__((always_inline))
  {
    CRGB c;
    hsv2rgb_rainbow( CHSV(hue, sat, val), c);
    *this = c;
    fixOverflow8 ();
    return *this;
  }

  /// allow assignment from just a Hue, saturation and value automatically at max.
  inline CRGB_d& setHue (uint8_t hue) __attribute__((always_inline))
  {
    return this->setHSV (hue, 255, 255);
  }

  ///conversion from PhillipsHue XY to RGB
  CRGB_d& setXY(float x, float y, uint8_t bri){
    int optimal_bri = bri;
    if (optimal_bri < 5) {
      optimal_bri = 5;
    }
    float Y = y;
    float X = x;
    float Z = 1.0f - x - y;

    // sRGB D65 conversion (Matrix)
    float r =  X * 3.2406f - Y * 1.5372f - Z * 0.4986f;
    float g = -X * 0.9689f + Y * 1.8758f + Z * 0.0415f;
    float b =  X * 0.0557f - Y * 0.2040f + Z * 1.0570f;


    // Apply gamma correction
    r = r <= 0.04045f ? r / 12.92f : pow((r + 0.055f) / (1.0f + 0.055f), 2.4f);
    g = g <= 0.04045f ? g / 12.92f : pow((g + 0.055f) / (1.0f + 0.055f), 2.4f);
    b = b <= 0.04045f ? b / 12.92f : pow((b + 0.055f) / (1.0f + 0.055f), 2.4f);

    float maxv = 0;// calc the maximum value of r g and b
    if (r > maxv) maxv = r;
    if (g > maxv) maxv = g;
    if (b > maxv) maxv = b;

    if (maxv > 0) {// only if maximum value is greater than zero, otherwise there would be division by zero
      r /= maxv;   // scale to maximum so the brightest light is always 1.0
      g /= maxv;   // brightness is computed later
      b /= maxv;
    }

    r = r < 0 ? 0 : r;// limit to min zero
    g = g < 0 ? 0 : g;
    b = b < 0 ? 0 : b;

    this->r = (r * optimal_bri);// scale by brightness
    this->g = (g * optimal_bri);
    this->b = (b * optimal_bri);
    fixOverflow8 ();
    return *this;
  }

  ///conversion from PhillipsHue HSB to RGB
  CRGB_d& setHSB(int hue, uint8_t sat, uint8_t bri){
    double      hh, p, q, t, ff, s, v;
    long        i;

    s = sat / 255.0;
    v = bri / 255.0;

    if (s <= 0.0) {
      this->r = v;
      this->g = v;
      this->b = v;
      fixOverflow8 ();
      return *this;
    }
    hh = hue;
    if (hh >= 65535.0) hh = 0.0;
    hh /= 11850.0;
    i = (long)hh;
    ff = hh - i;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * ff));
    t = v * (1.0 - (s * (1.0 - ff)));

    switch (i) {
    case 0:
      this->r = v * 255.0;
      this->g = t * 255.0;
      this->b = p * 255.0;
      break;
    case 1:
      this->r = q * 255.0;
      this->g = v * 255.0;
      this->b = p * 255.0;
      break;
    case 2:
      this->r = p * 255.0;
      this->g = v * 255.0;
      this->b = t * 255.0;
      break;

    case 3:
      this->r = p * 255.0;
      this->g = q * 255.0;
      this->b = v * 255.0;
      break;
    case 4:
      this->r = t * 255.0;
      this->g = p * 255.0;
      this->b = v * 255.0;
      break;
    case 5:
    default:
      this->r = v * 255.0;
      this->g = p * 255.0;
      this->b = q * 255.0;
      break;
    }
    fixOverflow8 ();
    return *this;
  }

  /// allow assignment from HSV color
  inline CRGB_d& operator= (const CHSV& rhs) __attribute__((always_inline))
  {
    CRGB c;
    hsv2rgb_rainbow( rhs, c);
    r = c.r;
    g = c.g;
    b = c.b;
    return *this;
  }

  /// allow assignment from 32-bit (really 24-bit) 0xRRGGBB color code
  inline CRGB_d& setColorCode (uint32_t colorcode) __attribute__((always_inline))
  {
    r = (colorcode >> 16) & 0xFF;
    g = (colorcode >>  8) & 0xFF;
    b = (colorcode >>  0) & 0xFF;
    return *this;
  }

  bool operator== (const CRGB_d c){
    if(r == c.r && g == c.g && b == c.b)return true;
    return false;
  }

  bool operator!= (const CRGB_d c){
    if(r != c.r || g != c.g || b != c.b)return true;
    return false;
  }

  /// add one RGB to another, saturating at 0xFF for each channel
  inline CRGB_d& operator+= (const CRGB_d& rhs )
  {
    r += rhs.r;
    g += rhs.g;
    b += rhs.b;
    fixOverflow8();
    return *this;
  }

  /// add a contstant to each channel, saturating at 0xFF
  /// this is NOT an operator+= overload because the compiler
  /// can't usefully decide when it's being passed a 32-bit
  /// constant (e.g. CRGB::Red) and an 8-bit one (CRGB::Blue)
  inline CRGB_d& addToRGB (uint8_t d )
  {
    r += d;
    g += d;
    b += d;
    fixOverflow8();
    return *this;
  }

  /// subtract one RGB from another, saturating at 0x00 for each channel
  inline CRGB_d& operator-= (const CRGB_d& rhs )
  {
    r -= rhs.r;
    g -= rhs.g;
    b -= rhs.b;
    fixOverflow8();
    return *this;
  }

  /// subtract a constant from each channel, saturating at 0x00
  /// this is NOT an operator+= overload because the compiler
  /// can't usefully decide when it's being passed a 32-bit
  /// constant (e.g. CRGB::Red) and an 8-bit one (CRGB::Blue)
  inline CRGB_d& subtractFromRGB(double d )
  {
    r -= d;
    g -= d;
    b -= d;
    fixOverflow8();
    return *this;
  }

  /// subtract a constant of '1' from each channel, saturating at 0x00
  inline CRGB_d& operator-- ()  __attribute__((always_inline))
  {
    subtractFromRGB(1);
    return *this;
  }

  /// subtract a constant of '1' from each channel, saturating at 0x00
  inline CRGB_d operator-- (int )  __attribute__((always_inline))
  {
    CRGB_d retval(*this);
    --(*this);
    retval = fixOverflow8 (retval);
    return retval;
  }

  /// add a constant of '1' from each channel, saturating at 0xFF
  inline CRGB_d& operator++ ()  __attribute__((always_inline))
  {
    addToRGB(1);
    return *this;
  }

  /// add a constant of '1' from each channel, saturating at 0xFF
  inline CRGB_d operator++ (int )  __attribute__((always_inline))
  {
    CRGB_d retval(*this);
    ++(*this);
    retval = fixOverflow8 (retval);
    return retval;
  }

  /// divide each of the channels by a constant
  inline CRGB_d& operator/= (double d )
  {
    r /= d;
    g /= d;
    b /= d;
    fixOverflow8();
    return *this;
  }

  /// multiply each of the channels by a constant,
  /// saturating each channel at 0xFF
  CRGB_d& operator*= (double d )
  {
    r *= d;
    g *= d;
    b *= d;
    fixOverflow8();
    return *this;
  }

  CRGB_d operator*(double d){
    CRGB_d c;
    c.r = r * d;
    c.g = g * d;
    c.b = b * d;
    c = fixOverflow8(c);
    return c;
  }

  CRGB_d operator/(double d){
    CRGB_d c;
    c.r = r / d;
    c.g = g / d;
    c.b = b / d;
    c = fixOverflow8(c);
    return c;
  }

  CRGB_d operator+(uint8_t d){
    CRGB_d c;
    c.r = r + d;
    c.g = g + d;
    c.b = b + d;
    c = fixOverflow8(c);
    return c;
  }

  CRGB_d operator+(CRGB_d d){
    CRGB_d c;
    c.r = r + d.r;
    c.g = g + d.g;
    c.b = b + d.b;
    c = fixOverflow8(c);
    return c;
  }

  CRGB_d operator-(uint8_t d){
    CRGB_d c;
    c.r = r - d;
    c.g = g - d;
    c.b = b - d;
    c = fixOverflow8(c);
    return c;
  }

  CRGB_d operator-(CRGB_d d){
    CRGB_d c;
    c.r = r - d.r;
    c.g = g - d.g;
    c.b = b - d.b;
    c = fixOverflow8(c);
    return c;
  }

  /// "or" operator brings each channel up to the higher of the two values
  inline CRGB_d& operator|= (const CRGB_d& rhs )
  {
    if( rhs.r > r) r = rhs.r;
    if( rhs.g > g) g = rhs.g;
    if( rhs.b > b) b = rhs.b;
    fixOverflow8 ();
    return *this;
  }

  /// "or" operator brings each channel up to the higher of the two values
  inline CRGB_d& operator|= (uint8_t d )
  {
    if( d > r) r = d;
    if( d > g) g = d;
    if( d > b) b = d;
    fixOverflow8 ();
    return *this;
  }

  /// "and" operator brings each channel down to the lower of the two values
  inline CRGB_d& operator&= (const CRGB_d& rhs )
  {
    if( rhs.r < r) r = rhs.r;
    if( rhs.g < g) g = rhs.g;
    if( rhs.b < b) b = rhs.b;
    fixOverflow8 ();
    return *this;
  }

  /// "and" operator brings each channel down to the lower of the two values
  inline CRGB_d& operator&= (uint8_t d )
  {
    if( d < r) r = d;
    if( d < g) g = d;
    if( d < b) b = d;
    fixOverflow8 ();
    return *this;
  }

  /// this allows testing a CRGB for zero-ness
  inline operator bool() const __attribute__((always_inline))
  {
    return r || g || b;
  }

  /// invert each channel
  inline CRGB_d operator- ()
  {
    CRGB_d retval;
    retval.r = 255 - r;
    retval.g = 255 - g;
    retval.b = 255 - b;
    retval = fixOverflow8 (retval);
    return retval;
  }

  static CRGB_d fixOverflow8(CRGB_d& c){
    CRGB_d r = c;//copy
    if(r.r > 255) r.r = 255;
    if(r.g > 255) r.g = 255;
    if(r.b > 255) r.b = 255;
    if(r.r < 0) r.r = 0;
    if(r.g < 0) r.g = 0;
    if(r.b < 0) r.b = 0;
    return r;
  }

  void fixOverflow8(){
    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;
    if(r < 0) r = 0;
    if(g < 0) g = 0;
    if(b < 0) b = 0;
  }
};

#endif //CRGB_D_H
