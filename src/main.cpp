#include <Arduino.h>
#include <AccelStepper.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

void updateScreen();

void updateSwitches();

int Stepper1Pulse = 25;  // **** for CNC shield
int Stepper1Direction = 26;  // **** for CNC shield
const byte enablePin = 27;   // **** for CNC shield
int speedpot = A0;

int x_lsPitch = 2; // Lead screw pitch in mm for calculations
int z_lsPitch = 2;
int xscale = 1; // Scaling factors for pulley gear reductions
int zscale = 3;

String axis = "X: ";

// Define some pins for user controls
int onoffPin = 3;
int rapidPin = 4;
int directionPin = 2;

// Motor stuff
int Motor1speed = 0;
int Drivespeed = 0;
int speedmin = 0;
int speedmax = 1000;
AccelStepper step1(1, Stepper1Pulse, Stepper1Direction);

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{
   // Start serial
   Serial.begin(115200); // ************** faster baud rate
   // Serial.println("Running: StepperDriverTest");

   step1.setMaxSpeed (speedmax);
   step1.setSpeed(0);
   step1.setAcceleration(500);
   pinMode(enablePin, OUTPUT);  // **** for CNC shield
   digitalWrite(enablePin, LOW);   // **** for CNC shield
   digitalWrite(Stepper1Direction, HIGH); // CW
   
   // Setup pins for controls
   pinMode( rapidPin, INPUT_PULLUP );
   pinMode( onoffPin, INPUT_PULLUP );
   pinMode( directionPin, INPUT_PULLUP );

   // Setup the screen
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
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

      Motor1speed = map((analogRead(speedpot)), 0, 4096, speedmin, speedmax); 

      updateSwitches();
      updateScreen();

      step1.setSpeed(Motor1speed);
      // Serial.println(Motor1speed);
      Serial.println(analogRead(speedpot));
   }
      step1.runSpeed();
}

void updateSwitches() // Function to update the states of the controls
{
    // Read the switch states
    int onoff = digitalRead(onoffPin);
    int rapid = digitalRead(rapidPin);
    int direction = digitalRead(directionPin);
  
    // Rapid logic
    if (rapid == LOW) {
        Motor1speed = speedmax;
    }

    // On/Off logic
    if (onoff == LOW) {
        Motor1speed = speedmin;
        digitalWrite(enablePin, HIGH);
    } else {
        digitalWrite(enablePin, LOW);
    }
    
    // Direction logic
    if (direction == LOW) {
        Motor1speed = Motor1speed * -1;
    }  
}

void updateScreen() // Update the OLED display
{
     int traverse = Motor1speed / x_lsPitch;

     display.clearDisplay();
     display.setCursor(0, 10);
     display.setTextSize(2);
     display.print(axis);
     display.print(traverse);
     display.setTextSize(1);
     display.print(" mm/min");
     display.display();
}