/*
  OAK RGB Controller by Rinié Romain
  Control an RGB PWM driver from a Digistump OAK board.
*/

#include <RGBLib.h>

/*****************************************************************
  USER SETTINGS: You can redefine the different output in this part.
 *****************************************************************/

int ledPin = 1;       // choose the pin for the status LED (Oak on-board LED)
int pushButtonInputPin = 10;   // choose the input pin (for a pushbutton)
int sensitiveButtonInputPin = 5; // choose the input pin (for a sensitive button with reserve input)

int redLedPin = 8;    //choose the pin for the RED LED channel
int greenLedPin = 7;  //choose the pin for the GREEN LED channel
int blueLedPin = 6;   //choose the pin for the BLUE LED channel
int whiteLedPin = 9;  //choose the pin for the WHITE LED channel

RGB rgb(200, 8, 7, 6, 9);


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
int sensitiveButtonValue = 0;       // variable for the actual push button status
int lastPushButtonValue = 0;   // variable for the previous push button value
int lastSensitiveButtonValue = 1;   // variable for the previous sensitive button value

/*
   LED ON/OFF Particle registered function
   @Return: return 1 if command was executed, -1 if command is not supported
   @Param command: on or off.
   on:  turn all led to the preset value
   off: turn all led off
*/
int ledToggleFunction(String command) {
  if (command == "on") {
    rgb.turnOn();
    return 1;
  }
  else if (command == "off") {
    rgb.turnOff();
    return 1;
  }
  else {
    return -1;
  }
}

/*
   Extraction from int from a string
   @Return: Extracted int value
   @Param startIndex: start index of the integer to extract
   @Param endIndex:   end   index of the integer to extract
   @Param commandIn:  string to extract the integer from
*/
int stringToInt(int startIndex, int endIndex, String commandIn) {
  return commandIn.substring(startIndex, endIndex).toInt();
}

/*
   LED Intensity Particle registered function
   @Return: return 1 if command was registered, -1 if command is not supported
   @Param command: intensity to set, from 000 to 255.
   0 or 00 won't be accepted, value should be 3 decimal, like 000 or 014 and not 0 or 14.
*/
int setIntensity(String command) {
  if (command.length() != 3) {
    return -1;
  }
  intensity = stringToInt(0, 3, command);
  rgb.setIntensity(intensity);
  rgb.turnOn();
  return 1;
}

/*
   LED value Particle registered function
   @Return: return 1 if command was registered, -1 if command is not supported
   @Param command: value to set, from 000000000000 to 255255255255 in the sequence RGBW.
   0 or 00 won't be accepted, value should be 3 decimal, like 000 or 014 and not 0 or 14.
*/
int setValue(String command) {
  if (command.length() != 12) {
    return -1;
  }
  redValue = stringToInt(0, 3, command);
  greenValue = stringToInt(3, 6, command);
  blueValue = stringToInt(6, 9, command);
  whiteValue = stringToInt(9, 12, command);
  rgb.setValue(redValue, greenValue, blueValue, whiteValue);
  rgb.turnOn();
  return 1;
}

void setup() {
  pinMode(ledPin, OUTPUT);        // Initialize the BUILTIN_LED pin as an output
  pinMode(redLedPin, OUTPUT);     // Initialize the RGB_RED_LED pin as an output
  pinMode(greenLedPin, OUTPUT);   // Initialize the RGB_GREEN_LED pin as an output
  pinMode(blueLedPin, OUTPUT);    // Initialize the RGB_BLUE_LED pin as an output
  pinMode(whiteLedPin, OUTPUT);   // Initialize the RGB_WHITE_LED pin as an output
  pinMode(pushButtonInputPin, INPUT);          //Initialize the input push button
  pinMode(sensitiveButtonInputPin, INPUT);          //Initialize the input push button

  Particle.function("led", ledToggleFunction);  // "led" function to turn the led on or off
  Particle.function("value", setValue);         // "value" function to set the led value
  Particle.function("intensity", setIntensity); // "intensity" function to set the led intensity
  Particle.variable("red", rgb._redValue);           // "red"   variable representing the red   led value (on 255)
  Particle.variable("green", rgb._greenValue);       // "green" variable representing the green led value (on 255)
  Particle.variable("blue", rgb._blueValue);         // "blue"  variable representing the blue  led value (on 255)
  Particle.variable("white", rgb._whiteValue);       // "white" variable representing the white led value (on 255)
  Particle.variable("inten", rgb._intensity);        // "inten" variable representing the led intensity (on 255)
  rgb.turnOff();
  lastPushButtonValue = digitalRead(pushButtonInputPin);
  lastSensitiveButtonValue = digitalRead(sensitiveButtonInputPin);
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
    sensitiveButtonValue = digitalRead(sensitiveButtonInputPin);

    if ((pushButtonValue == lastPushButtonValue && pushButtonValue == LOW)
        || (sensitiveButtonValue == lastSensitiveButtonValue && sensitiveButtonValue == HIGH)) { //Keep press, register in the loop count
      pushLoopCount = pushLoopCount + 1;
    }

    if (
      ((pushButtonValue == HIGH) && (sensitiveButtonValue == LOW))
      && (pushLoopCount <= numberOfLoopBeforeLedValueToogle)
      && ((pushButtonValue != lastPushButtonValue) || (sensitiveButtonValue != lastSensitiveButtonValue))
    ) { //Button just released and was in the previous status for less than 3 loop
      if (whiteValue != 0) {
        whiteValue = 0;
        rgb.turnOff();
      } else {
        whiteValue = 200;
        rgb.turnWhiteOn();
      }
    }

    if (pushLoopCount > numberOfLoopBeforeLedValueToogle) { // pressed for more than numberOfLoopBeforeLedValueToogle cycle, toogle color
      rgb.next();
    }

    if (pushLoopCount > numberOfLoopBeforeWifiSetup) { // pressed for more than numberOfLoopBeforeWifiSetup cycle, start wifi config
      Oak.rebootToConfig();
    }

    if ((pushButtonValue != lastPushButtonValue) || (sensitiveButtonValue != lastSensitiveButtonValue)) { //Pressed status changed, reset loop counter
      pushLoopCount = 0;
    }
    lastPushButtonValue = pushButtonValue;
    lastSensitiveButtonValue = sensitiveButtonValue;
  }
}
