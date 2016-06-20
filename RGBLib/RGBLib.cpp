/*
  RGBLib.h - Helper library to control RGBW led strip
  Create by Romain Rinié, June 06, 2016
*/

#include "Arduino.h"
#include "RGBLib.h"

RGB::RGB(int initialeIntensity, int redPin, int greenPin, int bluePin, int whitePin) {
	pinMode(redPin, OUTPUT);
	pinMode(greenPin, OUTPUT);
	pinMode(bluePin, OUTPUT);
	pinMode(whitePin, OUTPUT);
	
	_intensity = initialeIntensity;
	_redPin = redPin;
	_greenPin = greenPin;
	_bluePin = bluePin;
	_whitePin = whitePin;
	_maxIntensity = 1023;
	
	_targetColor = 1;
	_nIntensity = 0;
	_nDirection = 1;
	_whiteValue = 200;
}

void RGB::turnOn() {
	analogWrite(_redPin, getValueWithAppliedIntensity(_redValue));
	analogWrite(_greenPin, getValueWithAppliedIntensity(_greenValue));
	analogWrite(_bluePin, getValueWithAppliedIntensity(_blueValue));
	analogWrite(_whitePin, getWhiteValue(_whiteValue));
}

void RGB::setValue(int redValue, int greenValue, int blueValue, int whiteValue) {
	_redValue = redValue;
	_greenValue = greenValue;
	_blueValue = blueValue;
	_whiteValue = whiteValue;
}

void RGB::turnWhiteOn() {
	analogWrite(_whitePin, getWhiteValue(_whiteValue));
}

void RGB::turnOff() {
	analogWrite(_redPin, 0);
	analogWrite(_greenPin, 0);
	analogWrite(_bluePin, 0);
	analogWrite(_whitePin, 0);
}
void RGB::setIntensity(int intensity) {
	_intensity = intensity;
}
void RGB::setMaxIntensity(int maxIntensity) {
	_maxIntensity = maxIntensity;
}

#define MAX_INTENSITY 8
#define MIN_INTENSITY 0
#define MAX_TARGETCOLOR 7
void RGB::next() {
	if (_nDirection) {
		// Direction is positive, fading towards the color
		if (++_nIntensity >= MAX_INTENSITY) {
			// Maximum intensity reached
			_nIntensity = MAX_INTENSITY;  // Just in case
			_nDirection = 0;             // Now going to fade OUT
		} // else : nothing to do
	} else {
		if (--_nIntensity <= MIN_INTENSITY) {
			_nIntensity = MIN_INTENSITY; // Just in case
			// When we get back to black, find the next target color
			if (++_targetColor>MAX_TARGETCOLOR) 
				_targetColor=1;          // We'll skip fading in and out of black
			_nDirection = 1;             // Now going to fade IN
		} // else: nothing to do
	}

	// Compute the colors
	int colors[3];
	for (int i=0;i<3;i++) {
		// If the corresponding bit in targetColor is set, it's part of the target color
		colors[i] = (_targetColor & (1<<i)) ? (1<<_nIntensity) -1 : 0;
	}

	// Set the color
	//setColor(colors[0], colors[1], colors[2]);
	_redValue=colors[0];
	_greenValue=colors[1];
	_blueValue=colors[2];
	turnOn();
}

/*
 * Output value calculation
 * @Return: baseValue with applied intensity and maxIntensity
 * @Param baseValue: value between 0 and 255
 */
int RGB::getValueWithAppliedIntensity(int baseValue) {
  if (0 == baseValue) {
    return 0;
  }
  if (0 == _intensity) {
    return 0;
  }
  return (((baseValue*_intensity)/255)*_maxIntensity)/255;
}

/*
 * Output value calculation
 * @Return: baseValue with applied maxIntensity
 * @Param baseValue: value between 0 and 255
 */
int RGB::getWhiteValue(int baseValue) {
  if (0 == baseValue) {
    return 0;
  }
  return (baseValue*_maxIntensity)/255;
}
