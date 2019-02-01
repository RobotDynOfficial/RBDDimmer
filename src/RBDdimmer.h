#ifndef RBDDIMMER_H
#define RBDDIMMER_H

#include <stdlib.h>

#if   defined(ARDUINO_ARCH_AVR)
	#include "avr/RBDmcuAVR.h"
#elif defined(ARDUINO_ARCH_ESP32)
	#include "esp32/RBDmcuESP32.h"
#elif defined(ARDUINO_ARCH_ESP8266)
	#include "esp8266/RBDmcuESP8266.h"
#elif defined(ARDUINO_ARCH_SAMD)
	#include "samd/RBDmcuSAMD21.h"
#elif defined(ARDUINO_ARCH_SAM)
	#include "sam/RBDmcuSAM.h"
#elif defined(ARDUINO_ARCH_STM32F1)
	#include "stm32duino/STM32F1/RBDmcuSTM32F1.h"
#elif defined(ARDUINO_ARCH_STM32F4)
	#include "stm32duino/STM32F4/RBDmcuSTM32F4.h"
#else 
	#error "This library only supports boards with an AVR, ESP32, ESP8266, SAMD, SAM, STM32F1/F4 processor."
#endif

typedef enum
{
    NORMAL_MODE = 0,
    TOGGLE_MODE = 1
} DIMMER_MODE_typedef;

typedef enum
{
    OFF = false,
    ON = true
} ON_OFF_typedef;

#define ALL_DIMMERS 30

class dimmerLamp 
{         
    private:
        int current_num;
		int timer_num;
        bool toggle_state;
        int tog_current;
		
		void port_init(void);
		void timer_init(void);
		void ext_int_init(void);
		
    public:   
        uint16_t pulse_begin;
        int dimmer_pin;
        int tog_max;
        int tog_min;
		
#if !(defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD))
        int zc_pin;

        dimmerLamp(int user_dimmer_pin, int zc_dimmer_pin);
#else
		dimmerLamp(int user_dimmer_pin);
#endif
        void begin(DIMMER_MODE_typedef DIMMER_MODE, ON_OFF_typedef ON_OFF);
        void setPower(int power);
		int  getPower(void);
		void setState(ON_OFF_typedef ON_OFF);
        bool getState(void);
		void changeState(void);
        void setMode(DIMMER_MODE_typedef DIMMER_MODE);
        DIMMER_MODE_typedef getMode(void);
        void toggleSettings(int minValue, int maxValue);  
};

#endif
