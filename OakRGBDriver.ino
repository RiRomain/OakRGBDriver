/*
 OAK RGB Controller by Rinié Romain
 Control an RGB PWM driver from a Digistump OAK board.
*/
//#define TEMP_SENSOR //Uncomment/comment out to activate/deactivate the temperature sensor

#if defined TEMP_SENSOR
  #include <OneWire.h>
#endif

int ledPin = 1;       // choose the pin for the status LED
int pushButtonInputPin = 10;        // choose the input pin (for a pushbutton)
int pushButtonValue = 0;       // variable for reading the pin status
int lastPushButtonValue = 0;   // variable for keeping previous button value

#if defined TEMP_SENSOR
  int DS18S20_Pin = 2;    //choose the pin for the DS18B20 PIN
#endif

int redLedPin = 6;    //choose the pin for the RED LED
int greenLedPin = 7;  //choose the pin for the GREEN LED
int blueLedPin = 8;   //choose the pin for the BLUE LED
int whiteLedPin = 9;  //choose the pin for the WHITE LED

#if defined TEMP_SENSOR
  float tempC = 0;     // Temperature in degrees C
  char tempCstring[6]; // Used to store a temperature value as a string
  OneWire ds(DS18S20_Pin);
#endif

int redValue = 0;
int greenValue = 0;
int blueValue = 0;
int whiteValue = 0;
int intensity = 0;
int maxIntensity = 1023;

boolean ledAreOn = false;

int getValueWithAppliedIntensity(int baseValue) {
  if (0 == baseValue) {
    return 0;
  }
  if (0 == intensity) {
    return 0;
  }
  return (((baseValue*intensity)/255)*maxIntensity)/255;
}

void setPresetValueToOutput() {
  analogWrite(ledPin, getValueWithAppliedIntensity(redValue));
  analogWrite(redLedPin, getValueWithAppliedIntensity(redValue));
  analogWrite(greenLedPin, getValueWithAppliedIntensity(greenValue));
  analogWrite(blueLedPin, getValueWithAppliedIntensity(blueValue));
  analogWrite(whiteLedPin, getValueWithAppliedIntensity(whiteValue));
}

void turnAllLedOff() {
  analogWrite(ledPin, 0);
  analogWrite(redLedPin, 0);
  analogWrite(greenLedPin, 0);
  analogWrite(blueLedPin, 0);
  analogWrite(whiteLedPin, 0);
}

int ledToggleFunction(String command) {
  if (command=="on") {
        setPresetValueToOutput();
        ledAreOn = true;
        return 1;
    }
    else if (command=="off") {
        turnAllLedOff();
        ledAreOn = false;
        return 1;
    }
    else {
        return -1;
    }
}

int stringToInt(int startIndex, int endIndex, String commandIn) {
  return commandIn.substring(startIndex, endIndex).toInt();
}

int setIntensity(String command) {
  if (command.length() != 3) {
    return -1;  
  }
  intensity = stringToInt(0, 3, command);
  setPresetValueToOutput();
  return 1;
}

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
  
  Particle.function("led", ledToggleFunction);
  Particle.function("value", setValue);
  Particle.function("intensity", setIntensity);
  Particle.variable("red", redValue);
  Particle.variable("green", greenValue);
  Particle.variable("blue", blueValue);
  Particle.variable("white", whiteValue);
  Particle.variable("inten", intensity);
//  char bla[] = "test";
//  Particle.variable("test", bla);
  setPresetValueToOutput();
}

//Time storage is preffered over pause function to create a delay, avoiding by this way to slow down the rest of the execution
unsigned long previousLoopMillis = 0;     // Storage of the last time the push button value was read
const long buttonReadingInterval = 100; // Delay between 2 push button read value, button will have to be pressed at most 100mS to react
const long numberOfLoopBeforeWifiSetup = 100; //represent 10 secondes
unsigned long pushLoopCount = 0;

// the loop function runs over and over again forever
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousLoopMillis >= buttonReadingInterval) {
    // save the last time we read the push button value
    previousLoopMillis = currentMillis;  
    pushButtonValue = digitalRead(pushButtonInputPin);
    if (pushButtonValue != lastPushButtonValue) {
       if (pushButtonValue == LOW) { //Button pressed
            if (ledAreOn) {
              ledToggleFunction("off");
            } else {
              ledToggleFunction("on");
            }
        }
        lastPushButtonValue = pushButtonValue;
        pushLoopCount = 0; //Register the first pushed loop
    } else { //Value not changed
      if (pushButtonValue == LOW) { //Button still pressed
        pushLoopCount = pushLoopCount + 1;
      }
    }
    if (pushLoopCount > numberOfLoopBeforeWifiSetup) {
      Oak.rebootToConfig(); 
      pushLoopCount = 0;
    }
  }
}



