/*
  RGBLib.h - Helper library to control RGBW led strip
  Create by Romain Rinié, June 06, 2016
*/

#ifndef RGBLib_h
#define RGBLib_h
#include "Arduino.h"

class RGB
{
	public:
		RGB(int initialeIntensity, int redPin, int greenPin, int bluePin, int whitePin);
		void turnOn();
		void turnWhiteOn();
		void turnOff();
		void setIntensity(int intensity);			//set intensity from 0 to 255
		void setMaxIntensity(int maxIntensity);		// set max intensity: Arduino 255, ESP 1023
		void setValue(int redValue, int greenValue, int blueValue, int whiteValue);
		void next();
		int _redValue;
		int _greenValue;
		int _blueValue;
		int _whiteValue;
		int _intensity;
	private:
		int _redPin;
		int _greenPin;
		int _bluePin;
		int _whitePin;
		
		int _maxIntensity;
		int getValueWithAppliedIntensity(int basedValue);
		int getWhiteValue(int baseValue);
		
		int _targetColor;
		int _nIntensity;
		int _nDirection;
};

#endif