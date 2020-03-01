# Focuser
Focuser for Astronomy based on Arduino (usable by INDI)

This focuser is used with [INDIlib](http://indilib.org/)
It use the protcol Moonlite (As documented in the indilib project).
This focuer was based off of [Hans Astro Focuser ](https://github.com/Hansastro/Focuser), you should look into that one you may like it better. It has more features.

Mainly I just removed the temperature sensor, display and made it work with the Adafruit Motorshield v2(AMSv2). I had a few of the motorshields laying around so thought I would use one. The A4988 controller used in the original repository is a good stepper controller but I had the AMSv2 laying around.

Only 16 microstep speed is currently supported.

## Element used
- Arduino Uno ([link](https://www.arduino.cc/en/Main/ArduinoBoardUno))
- A bipolar stepper motor
- The [Moonlite Protocol](http://www.indilib.org/media/kunena/attachments/1/HighResSteppermotor107.pdf)
- A [AMSv2](https://www.adafruit.com/product/1438) Stepper motor controller
- [AMSv2 library](https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library)

## Internal library
- Moonlite. Used to code and decode the Moolite protocol
- StepperControl_AMSv2. Used to command the stepper motor

## Compilation
The files are location in the Arduino/Focuser folder. You do need to install the AMSv2 library for it to compile.
The Arduino IDE is recommanded for an easy compilation. 
To compile you need to open up Arduino/Focuser/Focuser.ino.
The Arduino IDE can be found hier: [link](https://www.arduino.cc/en/Main/Software)

Compile and transfer the program to the Arduino.

When you start your Indi server use the **indi_moonlite_focus** module to enable the focuser. 
The software provided by Moonlite on Windows is also usable.

