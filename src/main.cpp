#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP_FlexyStepper.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

void updateScreen();
void updateSwitches();
void selectAxis();
void runX();
void runZ();

// Define pins for the motors
int xPulse = 25;
int xDirection = 26;
const byte xEnable = 27;

int zPulse = 19;
int zDirection = 18;
const byte zEnable = 17;

int enable = 0;

// Misc variables for calculations
int x_lsPitch = 2; // Lead screw pitch in mm
int z_lsPitch = 4;
int lsPitch = 0;
int xscale = 1; // Scaling factors for pulley gear reductions in the form of X:1
int zscale = 3;
int scale = 0;

String axis = "Starting...";

// Define some pins for user controls
int onoffPin = 16;
int rapidPin = 4;
int directionPin = 2;
int speedpot = A0;
int onoff = 0;
int axisPin = 33;

// Set some healthy speed boundries (RPM)

int speedmin = 0;
int speedmax = 270;

// Motor settings
int DISTANCE_TO_TRAVEL_IN_STEPS = 2000;
int SPEED_IN_RPM = 0;
int ACCELERATION_IN_STEPS_PER_SECOND = 800;
int DECELERATION_IN_STEPS_PER_SECOND = 800;
int SPEED_IN_STEPS_PER_SECOND = 300;

int activeAxis = 0;

// create the stepper motor object
ESP_FlexyStepper xStepper;
ESP_FlexyStepper zStepper;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{
   // Start serial
   Serial.begin(115200); // ************** faster baud rate

   pinMode(xEnable, OUTPUT);  
   digitalWrite(xEnable, LOW);   
   digitalWrite(xDirection, HIGH); // CW

   pinMode(zEnable, OUTPUT);  
   digitalWrite(zEnable, LOW);   
   digitalWrite(zDirection, HIGH); // CW

   // Setup pins for controls
   pinMode( rapidPin, INPUT_PULLUP );
   pinMode( onoffPin, INPUT_PULLUP );
   pinMode( directionPin, INPUT_PULLUP );
   pinMode( axisPin, INPUT_PULLUP );

   // connect and configure the X motor to IO pins
   xStepper.connectToPins(xPulse, xDirection);
   // set the speed and acceleration rates for the X stepper motor
   xStepper.setStepsPerRevolution(200);
   xStepper.setSpeedInStepsPerSecond(SPEED_IN_STEPS_PER_SECOND);
   xStepper.setSpeedInRevolutionsPerSecond(SPEED_IN_RPM / 60);
   xStepper.setAccelerationInStepsPerSecondPerSecond(ACCELERATION_IN_STEPS_PER_SECOND);
   xStepper.setDecelerationInStepsPerSecondPerSecond(DECELERATION_IN_STEPS_PER_SECOND);

   // connect and configure the Z motor to IO pins
   zStepper.connectToPins(zPulse, zDirection);
   // set the speed and acceleration rates for the Z stepper motor
   zStepper.setStepsPerRevolution(200);
   zStepper.setSpeedInStepsPerSecond(SPEED_IN_STEPS_PER_SECOND);
   zStepper.setSpeedInRevolutionsPerSecond(SPEED_IN_RPM / 60);
   zStepper.setAccelerationInStepsPerSecondPerSecond(ACCELERATION_IN_STEPS_PER_SECOND);
   zStepper.setDecelerationInStepsPerSecondPerSecond(DECELERATION_IN_STEPS_PER_SECOND);

   xStepper.startAsService(0);  // Start Flexystepper as a sevice on core 0 to avoid being slowed down by the loop
   zStepper.startAsService(0);

   // Setup the screen
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
   Serial.println(F("SSD1306 allocation failed"));
   for(;;);
   }
   delay(2000);
   display.clearDisplay();

   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0, 10);
   // Display static text
   display.println("Hello, world!");
   display.display(); 
}

void loop()
{
   static unsigned long timer = 0;
   unsigned long interval = 100;
   if (millis() - timer >= interval)
   {
      timer = millis();

      SPEED_IN_RPM = map((analogRead(speedpot)), 0, 4096, speedmin, speedmax); 

      selectAxis();
      updateSwitches();
      updateScreen();

    if (activeAxis == LOW ) {
       runX();
    } else {
       runZ();
    }
   }
}

void updateSwitches() // Function to update the states of the controls
{
    // Read the switch states
    int onoff = digitalRead(onoffPin);
    int rapid = digitalRead(rapidPin);
    int direction = digitalRead(directionPin);

    if (SPEED_IN_RPM < 50) {
       onoff = LOW;  // Turns the stepper off if the potentiometer is set around 0
    }

    // Rapid logic
    if (rapid == LOW) {
        SPEED_IN_RPM = speedmax;
        DISTANCE_TO_TRAVEL_IN_STEPS = 10000;
    }

    // On/Off logic
    if (onoff == LOW) {
        SPEED_IN_RPM = speedmin;
        xStepper.setTargetPositionToStop();
        zStepper.setTargetPositionToStop();
        digitalWrite(xEnable, HIGH);
        digitalWrite(zEnable, HIGH);
    } else {
        digitalWrite(xEnable, LOW);
        digitalWrite(zEnable, LOW);
        DISTANCE_TO_TRAVEL_IN_STEPS = 10000;
    }
    
    // Direction logic
    if (direction == LOW) {
        DISTANCE_TO_TRAVEL_IN_STEPS = DISTANCE_TO_TRAVEL_IN_STEPS * -1;
    }
}

void updateScreen() // Update the OLED display
{
     int traverse = ((SPEED_IN_RPM / scale) * lsPitch);

     display.clearDisplay();
     display.setCursor(0, 10);
     display.setTextSize(2);
     display.print(axis);
     display.print(traverse);
     display.setTextSize(1);
     display.print(" mm/min");
     display.display();
}

void selectAxis() // Check for which axis is selected
{
   
    int axisSelect = digitalRead(axisPin);

    if (axisSelect == LOW) {
        axis = "Z: ";
        activeAxis = 1;
        scale = zscale;
        lsPitch = z_lsPitch;
        enable = zEnable;
        xStepper.setTargetPositionToStop();  // Stop the X axis as we're moving Z
    } else {
        axis = "X: ";
        activeAxis = 0;
        scale = xscale;
        lsPitch = x_lsPitch;
        enable = xEnable;
        zStepper.setTargetPositionToStop();  // Stop the z axis as we're moving X
    } 
}

void runX() // Run the X motor
{
    if (SPEED_IN_RPM < 50) {
       xStepper.setSpeedInRevolutionsPerSecond(SPEED_IN_RPM / 60);
       xStepper.setTargetPositionToStop();
    } else {
       xStepper.setSpeedInRevolutionsPerSecond(SPEED_IN_RPM / 60);
       xStepper.setTargetPositionRelativeInSteps(DISTANCE_TO_TRAVEL_IN_STEPS);
    }
}
void runZ() // Run the Z motor
{
    if (SPEED_IN_RPM < 50) {
       zStepper.setSpeedInRevolutionsPerSecond(SPEED_IN_RPM / 60);
       zStepper.setTargetPositionToStop();
    } else {
       zStepper.setSpeedInRevolutionsPerSecond(SPEED_IN_RPM / 60);
       zStepper.startJogging(DISTANCE_TO_TRAVEL_IN_STEPS);
    }
}