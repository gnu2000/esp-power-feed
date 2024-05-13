#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP_FlexyStepper.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

void updateScreen();
void updateSwitches();

int Stepper1Pulse = 25;  // **** for CNC shield
int Stepper1Direction = 26;  // **** for CNC shield
const byte enablePin = 27;   // **** for CNC shield
int speedpot = A0;

int x_lsPitch = 2; // Lead screw pitch in mm for calculations
int z_lsPitch = 4;
int xscale = 1; // Scaling factors for pulley gear reductions
int zscale = 3;

String axis = "X: ";

// Define some pins for user controls
int onoffPin = 17;
int rapidPin = 4;
int directionPin = 2;

int onoff = 0;

// Set some healthy speed boundries (RPM)

int speedmin = 0;
int speedmax = 270;

// Motor settings
int DISTANCE_TO_TRAVEL_IN_STEPS = 2000;
int SPEED_IN_RPM = 0;
int ACCELERATION_IN_STEPS_PER_SECOND = 800;
int DECELERATION_IN_STEPS_PER_SECOND = 800;
int SPEED_IN_STEPS_PER_SECOND = 300;

// create the stepper motor object
ESP_FlexyStepper stepper;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{
   // Start serial
   Serial.begin(115200); // ************** faster baud rate
   // Serial.println("Running: StepperDriverTest");

   pinMode(enablePin, OUTPUT);  // **** for CNC shield
   digitalWrite(enablePin, LOW);   // **** for CNC shield
   digitalWrite(Stepper1Direction, HIGH); // CW
   
   // Setup pins for controls
   pinMode( rapidPin, INPUT_PULLUP );
   pinMode( onoffPin, INPUT_PULLUP );
   pinMode( directionPin, INPUT_PULLUP );

   // connect and configure the stepper motor to its IO pins
   stepper.connectToPins(Stepper1Pulse, Stepper1Direction);
   // set the speed and acceleration rates for the stepper motor
   stepper.setStepsPerRevolution(200);
   stepper.setSpeedInStepsPerSecond(SPEED_IN_STEPS_PER_SECOND);
   stepper.setSpeedInRevolutionsPerSecond(SPEED_IN_RPM / 60);
   stepper.setAccelerationInStepsPerSecondPerSecond(ACCELERATION_IN_STEPS_PER_SECOND);
   stepper.setDecelerationInStepsPerSecondPerSecond(DECELERATION_IN_STEPS_PER_SECOND);

   stepper.startAsService(0);  // Start Flexystepper as a sevice on core 0 to avoid being slowed down by the loop

   // Setup the screen
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
   Serial.println(F("SSD1306 allocation failed"));
   for(;;);
   }
   Wire.setClock(1000000);                     // Teensy Steroid injection for the OLED
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

      updateSwitches();
      updateScreen();

      // Serial.println(SPEED_IN_RPM);
      // Serial.println(analogRead(speedpot));

    if (SPEED_IN_RPM < 50) {
       stepper.setSpeedInRevolutionsPerSecond(SPEED_IN_RPM / 60);
       stepper.setTargetPositionToStop();
    } else {
//       digitalWrite(enablePin, HIGH);   // Enable stepper
       stepper.setSpeedInRevolutionsPerSecond(SPEED_IN_RPM / 60);
       stepper.setTargetPositionRelativeInSteps(DISTANCE_TO_TRAVEL_IN_STEPS);
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
        stepper.setTargetPositionToStop();
        digitalWrite(enablePin, HIGH);
    } else {
        digitalWrite(enablePin, LOW);
        DISTANCE_TO_TRAVEL_IN_STEPS = 10000;
    }
    
    // Direction logic
    if (direction == LOW) {
        DISTANCE_TO_TRAVEL_IN_STEPS = DISTANCE_TO_TRAVEL_IN_STEPS * -1;
    }  
}

void updateScreen() // Update the OLED display
{
     int traverse = SPEED_IN_RPM * x_lsPitch;

     display.clearDisplay();
     display.setCursor(0, 10);
     display.setTextSize(2);
     display.print(axis);
     display.print(traverse);
     display.setTextSize(1);
     display.print(" mm/min");
     display.display();
}
