
The following library is used for work with dimmer, it gives ability to control large ammoun of dimmer by the means of hardware timer and external interrupt.
When used this libarary better discover pin table

 *  ---------------------- OUTPUT & INPUT Pin table ---------------------
 *  +---------------+-------------------------+-------------------------+
 *  |   Board       | INPUT Pin               | OUTPUT Pin              |
 *  |               | Zero-Cross              |                         |
 *  +---------------+-------------------------+-------------------------+
 *  | Leonardo       | D7 (NOT CHANGABLE)      | D0-D6, D8-D13           |
 *  +---------------+-------------------------+-------------------------+
 *  | Mega          | D2 (NOT CHANGABLE)      | D0-D1, D3-D70           |
 *  +---------------+-------------------------+-------------------------+
 *  | Uno           | D2 (NOT CHANGABLE)      | D0-D1, D3-D20           |
 *  +---------------+-------------------------+-------------------------+
 *  | ESP8266       | D1(IO5),    D2(IO4),    | D0(IO16),   D1(IO5),    |
 *  |               | D5(IO14),   D6(IO12),   | D2(IO4),    D5(IO14),   |
 *  |               | D7(IO13),   D8(IO15),   | D6(IO12),   D7(IO13),   |
 *  |               |                         | D8(IO15)                |
 *  +---------------+-------------------------+-------------------------+
 *  | ESP32         | 4(GPI36),   6(GPI34),   | 8(GPO32),   9(GP033),   |
 *  |               | 5(GPI39),   7(GPI35),   | 10(GPIO25), 11(GPIO26), |
 *  |               | 8(GPO32),   9(GP033),   | 12(GPIO27), 13(GPIO14), |
 *  |               | 10(GPI025), 11(GPIO26), | 14(GPIO12), 16(GPIO13), |
 *  |               | 12(GPIO27), 13(GPIO14), | 23(GPIO15), 24(GPIO2),  |
 *  |               | 14(GPIO12), 16(GPIO13), | 25(GPIO0),  26(GPIO4),  |
 *  |               | 21(GPIO7),  23(GPIO15), | 27(GPIO16), 28(GPIO17), |
 *  |               | 24(GPIO2),  25(GPIO0),  | 29(GPIO5),  30(GPIO18), |
 *  |               | 26(GPIO4),  27(GPIO16), | 31(GPIO19), 33(GPIO21), |
 *  |               | 28(GPIO17), 29(GPIO5),  | 34(GPIO3),  35(GPIO1),  |
 *  |               | 30(GPIO18), 31(GPIO19), | 36(GPIO22), 37(GPIO23), |
 *  |               | 33(GPIO21), 35(GPIO1),  |                         |
 *  |               | 36(GPIO22), 37(GPIO23), |                         |
 *  +---------------+-------------------------+-------------------------+
 *  | Arduino M0    | D7 (NOT CHANGABLE)      | D0-D6, D8-D13           |
 *  | Arduino Zero  |                         |                         |
 *  +---------------+-------------------------+-------------------------+
 *  | Arduino Due   | D0-D53                  | D0-D53                  |
 *  +---------------+-------------------------+-------------------------+
 *  | STM32         | PA0-PA15,PB0-PB15       | PA0-PA15,PB0-PB15       |
 *  | Black Pill    | PC13-PC15               | PC13-PC15               |
 *  | BluePill      |                         |                         |
 *  | Etc...        |                         |                         |
 *  +---------------+-------------------------+-------------------------+

This library can simplify user code with following functions:

    1. Function dimmerLamp - this function initializes the number of operating pin and is defined by the user
    Example: 
        a.dimmerLamp dimmer(4);    dimmer output DIM/PSM is initialized on the pin 4 for the bords WITHOUT changable ZERO-CROSS input pin (AVR, Arduino M0/Zero)
        b.dimmerLamp dimmer(4, 2);  dimmer output DIM/PSM is initialized on the pin 4 and zero-cross initialized on pin 2. Only for boards whith changable zero-cross(ESP32, ESP8266, Arduino Due)

    2. Function begin port initialization, timer and external interrupt from zero-cross.
    Example:
    dimmer.begin(NORMAL_MODE, ON/OFF); port initialization, work mode choice, ON/OFF. 
    Parameter 1: dimmer working modes consist of two choices - NORMAL_MODE and TOGGLE_MODE 
        a. NORMAL_MODE to make dimmer work in defined value from 0 to 100 (%) (integer)
            Example of this mode located in \RBDdimmer\examples\SimpleDimmer
        b. TOGGLE_MODE smooth change of dimming value up or down in a defined range. This solutions implies change of dimming values by means of hardware timer, without using the cycle code.
            Example of this mode located in \RBDdimmer\examples\SimpleToggleDimmer

    Parameter 2: ON/OFF.
        a. ON - turns timer ON, allows to use dimmer.
        b. OFF - turns timer parameters OFF, prevents the use of dimmer.

    3. Function setPower sets dimming value from 0 to 100%
        Example: dimmer.setPower(90);

    4. Function getPower to display current dimming value
        Example: Serial.print(dimmer.getPower()); Result 0~100 int
   
    5. Function setMode sets and changes the work mode (NORMAL_MODE and TOGGLE_MODE)
                dimmer.setMode(NORMAL_MODE/TOGGLE_MODE)

    6. Function getMode displays values of current work mode
        Example: Serial.print(dimmer.getPower());  Result 0 (NORMAL_MODE) or 1 (TOGGLE_MODE)
   
    7. Function setState sets dimming state ON/OFF
        Example: dimmer.setState(ON); delay(100); dimmer.setState(OFF);
   
    8. Function getState displays current state of dimmer
        Serial.print(dimmer.getState()); Result 0 (OFF) or 1 (ON)
   
    9. Function changeState changes dimmer state to the opposite one 
        Пример dimmer.setState(ON); delay(100); dimmer.changeState; delay(100);
  
    10. Function toggleSettings smooth change of dimming value up or down in a defined range
        Example located in \RBDdimmer\examples\SimpleToggleDimmer
