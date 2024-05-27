
# ESP Power Feed

This project is designed to control a power feed system for a small benchtop milling machine (in my case a Warco WM-14). The motors used are a pair of NEMA 23s and DM542-T drivers.

The program makes use of the [ESP-FlexyStepper](https://github.com/pkerspe/ESP-FlexyStepper) library, which has the killer feature of being able to run the steppers on a different core to the main loop. I also wanted to use a small OLED screen module to provide feedback on axis speeds.

Standard disclaimer - I'm no coder, so this whole thing is kinda cobbled together in VScode and PlatformIO. But it does work for me, and so I hope it will be useful for you.
## Hardware used and pinouts

This is not an exhaustive guide to building the hardware, but I will list the parts I used and give the pinouts for the ESP32 below:

***Parts List:***

- 24V Power Supply
- 24V–5V DC–DC Convertor - _for running the ESP32_
- NEMA 23 Stepper Motor & appropriate driver ×2
- ESP32 Development Board _(Mine is branded AZ-Delivery)_
- ESP32 Screw Terminal Prototyping Board _(Just makes the connections easier)_
- SSD1306 128×32 OLED Screen Module
- 10k Potentiometer
- 3 Position Industrial Rotary Switch* - _for engaging the feed_
- Rotary Selector Switch - _for selecting the axis_
- Momentary Push-to-Make button - _for the Rapid feed_
- Power Switch & Small Emergency Stop Button - _Wire in series across the 24V supply to make sure you can kill the motors fast_
- Fuse Holder & appropriately sized Fuse
- Some kind of enclosure - _I used a cheap industrial switch panel that's probably a size too small_

*Note on the rotary switch - I have this wired so that in the middle position (indicated 0) the ON/OFF pin is pulled low, and in the right position the DIRECTION pin is pulled low.

***Pinout Diagram***

Insert diagram here

| Pin     | Connection                       | Code Variable |
| ------- | -------------------------------- | ------------- |
| 5V      | +5V In                           |               |
| GND     | GND                              |               |
| 3V3     | Potentiometer VCC, Screen VCC    |               |
| VP (A0) | Potentiometer Output             | `speedpot`      |
| 2       | Direction Switch                 | `directionPin`  |
| 4       | Rapid Button                     | `rapidPin`      |
| 16      | On/Off Switch (not power on/off) | `onoffPin`      |
| 17      | Z Enable                         | `zEnable`       |
| 18      | Z Direction                      | `zDirection`    |
| 19      | Z Step                           | `zPulse`        |
| 21      | Screen SDA                       |               |
| 22      | Screen SCL                       |               |
| 25      | X Step                           | `xPulse`        |
| 26      | X Direction                      | `xDirection`    |
| 27      | X Enable                         | `xEnable`       |
| 33      | Axis Select Switch               | `axisPin`       |





## FAQ

#### Why are you using Stepper motors?

It's a good question - using regular DC motors would be orders of magnitude easier. Some way down the line I do hope to do a CNC conversion, so I went with steppers now so I wouldn't have to swap motors or make new brackets. Also, fun!

#### Why not Arduino?

Horsepower mostly - the original single-axis version of this project used an Arduino Nano but there just isn't enough compute power to run both motors and update the OLED screen. The ESP32 has a wonderful advantage of a second core to take the up the load.




## Acknowledgements

 - [ESP-FlexyStepper](https://github.com/pkerspe/ESP-FlexyStepper) for being a very cool library and making this whole thing possible.
 - [Awesome README](https://github.com/matiassingers/awesome-readme)


