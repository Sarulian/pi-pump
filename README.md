# pi-pump
Raspberry Pi powered water tank control system.

Using an [MB7052 ultrasonic sensor](https://www.maxbotix.com/Ultrasonic_Sensors/MB7052.htm) mounted in the lid of the tank to monitor water level and an [Adafruit relay](https://www.adafruit.com/product/3191) to control a 100W AC pump.

## Recommendations
* Format the resolution of whatever screen you use to show only ~6 lines of the terminal at a time.
* Boot pi without GUI.
* After compiling add ~pi-pump/timing to .bashrc to run at boot.
* This code uses the [WiringPi pin definitions](https://pinout.xyz/) (numbers shown in tooltip on pins).
