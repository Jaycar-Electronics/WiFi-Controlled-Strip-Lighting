
#include "lightEffects.h"
#include "Adafruit_NeoPixel.h"
#include <Arduino.h> //generic .cpp files will need arduino header.

// Simple defines to extract colour information from color packet.
#define RED(c) (c >> 16 & 0xff)
#define GREEN(c) (c >> 8 & 0xff)
#define BLUE(c) (c & 0xff)

/*
 * lightEffects.cpp -
 * This is the main implementation file for lighting effects for your WiFi-Controlled-Strip-lighting
 * 
 * Use lightEffects.h file to declare new effects, then use this to implement it; 
 * 
 * This will love to be refactored into a proper interface down the line.
*/

// In general, there's a delay before each calling of the function,
// as specified by the "rate" through the phone-app;

void solidEffect(Adafruit_NeoPixel &light, long colorOne, long colorTwo, LightEffectMix &mixer)
{
  const int n = light.numPixels();

  const short redStep = (RED(colorOne) - RED(colorTwo)) / n;
  const short greenStep = (GREEN(colorOne) - GREEN(colorTwo)) / n;
  const short blueStep = (BLUE(colorOne) - BLUE(colorTwo)) / n;

  switch (mixer)
  {
  case Mix:
    for (int i = 0; i < n; i++)
    {
      light.setPixelColor(i, RED(colorOne) - redStep * i,
                          GREEN(colorOne) - greenStep * i,
                          BLUE(colorOne) - blueStep * i);
    }
    break;

  case Alternate:
    for (int i = 0; i < n; i++)
    {
      if (i % 2)
        light.setPixelColor(i, colorOne);
      else
        light.setPixelColor(i, colorTwo);
    }
    break;

  case Single:
  default:
    for (int i = 0; i < n; i++)
    {
      light.setPixelColor(i, colorOne);
    }
    break;
  }
}

void rainbowEffect(Adafruit_NeoPixel &light, long &lastColour)
{
  const int n = light.numPixels();
  for (int i = 0; i < n; i++)
  {
    int hue = lastColour + (i * 65536L / n);

    light.setPixelColor(i, light.gamma32(light.ColorHSV(lastColour)));
    lastColour = hue & 0xFFFF;
  }
}