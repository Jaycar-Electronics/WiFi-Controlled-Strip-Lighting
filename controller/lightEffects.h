#ifndef __LIGHT_EFFECTS_H__
#define __LIGHT_EFFECTS_H__

//Definition file for different light effects.

#include "customTypes.h"
#include "Adafruit_NeoPixel.h"

void solidEffect(Adafruit_NeoPixel &light,long one, long two, LightEffectMix &mixer);
void rainbowEffect(Adafruit_NeoPixel &light, long& lastColour);
//void flashEffect(LightEffectColour &one, LightEffectColour &two, LightEffectMix &mixer);

//void fadeEffect(LightEffectColour &one, LightEffectColour &two, LightEffectMix &mixer);


#endif