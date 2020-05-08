/*
  JoystickMouseControl

 Controls the mouse from a joystick on an Arduino Leonardo, Micro or Due.
 Uses a pushbutton to turn on and off mouse control, and
 a second pushbutton to click the left mouse button

 Hardware:
 * 2-axis joystick connected to pins A0 and A1
 * pushbuttons connected to pin D2 and D3

 The mouse movement is always relative. This sketch reads
 two analog inputs that range from 0 to 1023 (or less on either end)
 and translates them into ranges of -6 to 6.
 The sketch assumes that the joystick resting values are around the
 middle of the range, but that they vary within a threshold.

 WARNING:  When you use the Mouse.move() command, the Arduino takes
 over your mouse!  Make sure you have control before you use the command.
 This sketch includes a pushbutton to toggle the mouse control state, so
 you can turn on and off mouse control.

 created 15 Sept 2011
 updated 28 Mar 2012
 by Tom Igoe

 this code is in the public domain
 */
 
/*
Jegyzet!!!
5-os portra rá van kötve a LED

*/ 

// set pin numbers for switch, joystick axes, and LED:

//8
const int switchPin = 3;      // atirtam az eger gombjara.
//const int mouseButton = 3;    // input pin for the mouse pushButton
const int xAxis = A0;         // joystick X axis
const int yAxis = A1;         // joystick Y axis
const int ledPin = 5;         // Mouse control LED

// parameters for reading the joystick:
int range = 12;               // output range of X or Y movement
int responseDelay = 5;        // response delay of the mouse, in ms
int threshold = range / 4;    // resting threshold
int center = range / 2;       // resting position value

boolean isMouseMoved = false;
int lastSwitchState = LOW;        // previous switch state
boolean isPressed = false;


boolean mouseIsActive = false;    // whether or not to control the mouse

const int mouseModeOff      = 0; //OFF
const int mouseModeAutodesk = 1; //Autodesk Fusion 360
const int mouseModeSketchup = 2; //Sketchup
int mouseMode = mouseModeOff;


//file:///C:/Progs/arduino-1.6.4/reference/arduino.cc/en/Reference/KeyboardModifiers.html
char modKey = KEY_LEFT_SHIFT;

//MouseMode-hoz értelmezve
char modKeys[] = {0, KEY_LEFT_SHIFT, 0};


void setup() {
  pinMode(switchPin, INPUT);       // the switch pin
  pinMode(ledPin, OUTPUT);         // the LED pin
  // take control of the mouse:
  Mouse.begin();
  Keyboard.begin();
}



void loop() {
  int switchState = digitalRead(switchPin);
  if (switchState != lastSwitchState) {
    if (switchState == HIGH) {
      mouseMode++;
      if (mouseMode > 2){
        mouseMode = 0;
      }
      
      if (mouseMode == 0){
        mouseIsActive = false;
      } else {
        mouseIsActive = true;
      }

      //régi implementacio
      //mouseIsActive = !mouseIsActive;
      digitalWrite(ledPin, mouseIsActive);
    }
  }
  
  // save switch state for next comparison:
  lastSwitchState = switchState;

  // read and scale the two axes:
  int xReading = readAxis(A0) / 5;
  int yReading = readAxis(A1) / 5;

  isMouseMoved = xReading != 0 || yReading != 0;


  // if the mouse control state is active, move the mouse:
  if (mouseIsActive && isMouseMoved) {
    char modKey = modKeys[mouseMode];
    
    if (modKey != 0){
      Keyboard.press(modKey);
      delay(2);
    }
    
    Mouse.move(xReading, yReading, 0);

    Mouse.press(MOUSE_MIDDLE);
    isPressed = true;
  } else {
    if (isPressed){
      Mouse.release(MOUSE_MIDDLE);
      isPressed = false;
      Keyboard.release(modKey);
    }
  }


  delay(responseDelay);
}



/*
  reads an axis (0 or 1 for x or y) and scales the
 analog input range to a range from 0 to <range>
 */

int readAxis(int thisAxis) {
  // read the analog input:
  int reading = analogRead(thisAxis);

  // map the reading from the analog input range to the output range:
  reading = map(reading, 0, 1023, 0, range);

  // if the output reading is outside from the
  // rest position threshold,  use it:
  int distance = reading - center;

  if (abs(distance) < threshold) {
    distance = 0;
  }

  // return the distance for this axis:
  return distance;
}



