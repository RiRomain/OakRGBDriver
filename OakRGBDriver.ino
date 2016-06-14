/*
 OAK RGB Controller by Rinié Romain
 Control an RGB PWM driver from a Digistump OAK board.
*/

/*****************************************************************
 USER SETTINGS: You can redefine the different output in this part.
 *****************************************************************/

int ledPin = 1;       // choose the pin for the status LED (Oak on-board LED)
int pushButtonInputPin = 10;   // choose the input pin (for a pushbutton)

int redLedPin = 8;    //choose the pin for the RED LED channel
int greenLedPin = 7;  //choose the pin for the GREEN LED channel
int blueLedPin = 6;   //choose the pin for the BLUE LED channel
int whiteLedPin = 9;  //choose the pin for the WHITE LED channel


/*****************************************************************
 PROGRAMM VARIABLE: Do not modified following value.
 *****************************************************************/
int redValue = 0;
int greenValue = 0;
int blueValue = 0;
int whiteValue = 0;
int intensity = 200;      //Start with an intensity of 200 to avoid having to reset it on power loss.
int maxIntensity = 1023;  //Max output for the PWM, 

int pushButtonValue = 0;       // variable for the actual push button status
int lastPushButtonValue = 0;   // variable for the previous push button value

/*
 * Output value calculation
 * @Return: baseValue with applied intensity and maxIntensity
 * @Param baseValue: value between 0 and 255
 */
int getValueWithAppliedIntensity(int baseValue) {
  if (0 == baseValue) {
    return 0;
  }
  if (0 == intensity) {
    return 0;
  }
  return (((baseValue*intensity)/255)*maxIntensity)/255;
}

/*
 * Output value calculation
 * @Return: baseValue with applied maxIntensity
 * @Param baseValue: value between 0 and 255
 */
int getWhiteValue(int baseValue) {
  if (0 == baseValue) {
    return 0;
  }
  return (baseValue*maxIntensity)/255;
}

/*
 * Turn on all the led to the actual set value
 */
void setPresetValueToOutput() {
  analogWrite(ledPin, getValueWithAppliedIntensity(intensity));
  analogWrite(redLedPin, getValueWithAppliedIntensity(redValue));
  analogWrite(greenLedPin, getValueWithAppliedIntensity(greenValue));
  analogWrite(blueLedPin, getValueWithAppliedIntensity(blueValue));
  analogWrite(whiteLedPin, getWhiteValue(whiteValue));
}

/*
 * Turn off all the led
 */
void turnAllLedOff() {
  analogWrite(ledPin, 0);
  analogWrite(redLedPin, 0);
  analogWrite(greenLedPin, 0);
  analogWrite(blueLedPin, 0);
  analogWrite(whiteLedPin, 0);
}

/*
 * LED ON/OFF Particle registered function
 * @Return: return 1 if command was executed, -1 if command is not supported
 * @Param command: on or off. 
 * on:  turn all led to the preset value
 * off: turn all led off
 */
int ledToggleFunction(String command) {
  if (command=="on") {
        setPresetValueToOutput();
        return 1;
    }
    else if (command=="off") {
        whiteValue = 0;
        turnAllLedOff();
        return 1;
    }
    else if (command=="pushOn") {
      whiteValue = 200;
        analogWrite(whiteLedPin, getWhiteValue(whiteValue));
        analogWrite(ledPin, getWhiteValue(whiteValue));
        return 1;
    }
    else {
        return -1;
    }
}

/*
 * Extraction from int from a string
 * @Return: Extracted int value
 * @Param startIndex: start index of the integer to extract
 * @Param endIndex:   end   index of the integer to extract
 * @Param commandIn:  string to extract the integer from
 */
int stringToInt(int startIndex, int endIndex, String commandIn) {
  return commandIn.substring(startIndex, endIndex).toInt();
}

/*
 * LED Intensity Particle registered function
 * @Return: return 1 if command was registered, -1 if command is not supported
 * @Param command: intensity to set, from 000 to 255. 
 * 0 or 00 won't be accepted, value should be 3 decimal, like 000 or 014 and not 0 or 14.
 */
int setIntensity(String command) {
  if (command.length() != 3) {
    return -1;  
  }
  intensity = stringToInt(0, 3, command);
  setPresetValueToOutput();
  return 1;
}

/*
 * LED value Particle registered function
 * @Return: return 1 if command was registered, -1 if command is not supported
 * @Param command: value to set, from 000000000000 to 255255255255 in the sequence RGBW. 
 * 0 or 00 won't be accepted, value should be 3 decimal, like 000 or 014 and not 0 or 14.
 */
int setValue(String command) {
  if (command.length() != 12) {
    return -1;
  }
  redValue = stringToInt(0, 3, command);
  greenValue = stringToInt(3, 6, command);
  blueValue = stringToInt(6, 9, command);
  whiteValue = stringToInt(9, 12, command);
  setPresetValueToOutput();
  return 1;
}

void setup() {
  pinMode(ledPin, OUTPUT);        // Initialize the BUILTIN_LED pin as an output
  pinMode(redLedPin, OUTPUT);     // Initialize the RGB_RED_LED pin as an output
  pinMode(greenLedPin, OUTPUT);   // Initialize the RGB_GREEN_LED pin as an output
  pinMode(blueLedPin, OUTPUT);    // Initialize the RGB_BLUE_LED pin as an output
  pinMode(whiteLedPin, OUTPUT);   // Initialize the RGB_WHITE_LED pin as an output
  pinMode(pushButtonInputPin, INPUT);          //Initialize the input push button
  
  Particle.function("led", ledToggleFunction);  // "led" function to turn the led on or off
  Particle.function("value", setValue);         // "value" function to set the led value
  Particle.function("intensity", setIntensity); // "intensity" function to set the led intensity
  Particle.variable("red", redValue);           // "red"   variable representing the red   led value (on 255)
  Particle.variable("green", greenValue);       // "green" variable representing the green led value (on 255)
  Particle.variable("blue", blueValue);         // "blue"  variable representing the blue  led value (on 255)
  Particle.variable("white", whiteValue);       // "white" variable representing the white led value (on 255)
  Particle.variable("inten", intensity);        // "inten" variable representing the led intensity (on 255)
  setPresetValueToOutput();
  lastPushButtonValue = digitalRead(pushButtonInputPin);
}

//Time storage is preffered over pause function to create a delay, avoiding by this way to slow down the rest of the execution
unsigned long previousLoopMillis = 0;     // Storage of the last time the push button value was read
const long buttonReadingInterval = 100; // Delay between 2 push button read value, button will have to be pressed at most 100mS to react
const long numberOfLoopBeforeLedValueToogle = 3; // represent 300mS, after that the Oak will start to go through all available color
const long numberOfLoopBeforeWifiSetup = 1000; //represent 100 secondes, after which the Oak will go in wifi config mode
unsigned long pushLoopCount = 0;

// the loop function runs over and over again forever
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousLoopMillis >= buttonReadingInterval) {
    // save the last time we read the push button value

    
    previousLoopMillis = currentMillis;  
    pushButtonValue = digitalRead(pushButtonInputPin);

    if(pushButtonValue == lastPushButtonValue && pushButtonValue == LOW) { //Keep press, register in the loop count
        pushLoopCount = pushLoopCount + 1;
    }

    if ((pushButtonValue == HIGH) && (pushLoopCount <= numberOfLoopBeforeLedValueToogle) && (pushButtonValue != lastPushButtonValue)) { //Button just released and was in the previous status for less than 3 loop 
        if (whiteValue!= 0) {
          ledToggleFunction("off");
        } else {
          ledToggleFunction("pushOn");
        }
    }
    
    if (pushLoopCount > numberOfLoopBeforeLedValueToogle) { // pressed for more than numberOfLoopBeforeLedValueToogle cycle, toogle color
      whiteValue=0; //Turn off white channel as we just turned it on
      toogleColor();
    }
    
    if (pushLoopCount > numberOfLoopBeforeWifiSetup) { // pressed for more than numberOfLoopBeforeWifiSetup cycle, start wifi config
      Oak.rebootToConfig(); 
    }

    if (pushButtonValue != lastPushButtonValue) { //Pressed status changed, reset loop counter
      pushLoopCount = 0;
    }
    lastPushButtonValue = pushButtonValue;
  }
}


#define MAX_INTENSITY 8
#define MIN_INTENSITY 0
#define MAX_TARGETCOLOR 7

int targetColor = 1;
int nIntensity = 0;
int nDirection = 1;         // When direction is 1 we fade towards the color (fade IN)
                            // when 0 we fade towards black (fade OUT)

  void toogleColor() {
        // Update the intensity value
        if (nDirection) {
            // Direction is positive, fading towards the color
            if (++nIntensity >= MAX_INTENSITY) {
                // Maximum intensity reached
                nIntensity = MAX_INTENSITY;  // Just in case
                nDirection = 0;             // Now going to fade OUT
            } // else : nothing to do
        } else {
            if (--nIntensity <= MIN_INTENSITY) {
                nIntensity = MIN_INTENSITY; // Just in case
                // When we get back to black, find the next target color
                if (++targetColor>MAX_TARGETCOLOR) 
                    targetColor=1;          // We'll skip fading in and out of black
                nDirection = 1;             // Now going to fade IN
            } // else: nothing to do
        }

        // Compute the colors
        int colors[3];
        for (int i=0;i<3;i++) {
            // If the corresponding bit in targetColor is set, it's part of the target color
            colors[i] = (targetColor & (1<<i)) ? (1<<nIntensity) -1 : 0;
        }

        // Set the color
        //setColor(colors[0], colors[1], colors[2]);
        redValue=colors[0];
        greenValue=colors[1];
        blueValue=colors[2];
        setPresetValueToOutput();
        }



