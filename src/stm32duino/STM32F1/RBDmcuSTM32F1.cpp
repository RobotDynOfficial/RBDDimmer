#if defined(ARDUINO_ARCH_STM32F1)

#include "RBDmcuSTM32F1.h"
#define __cplusplus

int pulseWidth = 2;
volatile int current_dim = 0;
int all_dim = 3;
int rise_fall = true;
char user_zero_cross = '0';

static int toggleCounter = 0;
static int toggleReload = 25;

static dimmerLamp* dimmer[ALL_DIMMERS];
static uint16_t dimPower[ALL_DIMMERS];
static uint16_t dimOutPin[ALL_DIMMERS];
static uint16_t dimZCPin[ALL_DIMMERS];
static uint16_t zeroCross[ALL_DIMMERS];
static DIMMER_MODE_typedef dimMode[ALL_DIMMERS];
static ON_OFF_typedef dimState[ALL_DIMMERS];
static uint16_t dimCounter[ALL_DIMMERS];
static uint16_t dimPulseBegin[ALL_DIMMERS];
static uint16_t togMax[ALL_DIMMERS];
static uint16_t togMin[ALL_DIMMERS];
static bool togDir[ALL_DIMMERS];

dimmerLamp::dimmerLamp(int user_dimmer_pin, int zc_dimmer_pin):
	dimmer_pin(user_dimmer_pin),
	zc_pin(zc_dimmer_pin)
{
	current_dim++;
	dimmer[current_dim-1] = this;
	current_num = current_dim-1;
	toggle_state = false;
	
	dimPulseBegin[current_dim-1] = 1;
	dimOutPin[current_dim-1] = user_dimmer_pin;
	dimZCPin[current_dim-1] = zc_dimmer_pin;
	dimCounter[current_dim-1] = 0;
	zeroCross[current_dim-1] = 0;
	dimMode[current_dim-1] = NORMAL_MODE;
	togMin[current_dim-1] = 0;
	togMax[current_dim-1] = 1;
	pinMode(user_dimmer_pin, OUTPUT);	
}

void dimmerLamp::timer_init(void)
{
	Timer3.setMode(TIMER_CH1, TIMER_OUTPUTCOMPARE);
    Timer3.setPeriod(95); // in microseconds
    Timer3.setCompare(TIMER_CH1, 1);      // overflow might be small
    Timer3.attachInterrupt(TIMER_CH1, onTimerISR);
}

void dimmerLamp::ext_int_init(void) 
{
	int inPin = dimZCPin[this->current_num];
	pinMode(inPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(inPin), isr_ext, RISING);
}

void dimmerLamp::begin(DIMMER_MODE_typedef DIMMER_MODE, ON_OFF_typedef ON_OFF)
{
	dimMode[this->current_num] = DIMMER_MODE;
	dimState[this->current_num] = ON_OFF;
	timer_init();
	ext_int_init();	
}

void dimmerLamp::setPower(int power)
{	
	if (power >= 99) 
	{
		power = 99;
	}
	dimPower[this->current_num] = power;
	dimPulseBegin[this->current_num] = powerBuf[power];
	
	delay(1);
}

int dimmerLamp::getPower(void)
{
	if (dimState[this->current_num] == ON)
		return dimPower[this->current_num];
	else return 0;
}

void dimmerLamp::setState(ON_OFF_typedef ON_OFF)
{
	dimState[this->current_num] = ON_OFF;
}

bool dimmerLamp::getState(void)
{
	bool ret;
	if (dimState[this->current_num] == ON) ret = true;
	else ret = false;
	return ret;
}

void dimmerLamp::changeState(void)
{
	if (dimState[this->current_num] == ON) dimState[this->current_num] = OFF;
	else 
		dimState[this->current_num] = ON;
}

DIMMER_MODE_typedef dimmerLamp::getMode(void)
{
	return dimMode[this->current_num];
}

void dimmerLamp::setMode(DIMMER_MODE_typedef DIMMER_MODE)
{
	dimMode[this->current_num] = DIMMER_MODE;
}

void dimmerLamp::toggleSettings(int minValue, int maxValue)
{
	if (maxValue > 99) 
	{
    	maxValue = 99;
	}
	if (minValue < 1) 
	{
    	minValue = 1;
	}
	dimMode[this->current_num] = TOGGLE_MODE;
	togMin[this->current_num] = powerBuf[maxValue];
	togMax[this->current_num] = powerBuf[minValue];

	toggleReload = 50;
}
 
void isr_ext(void)
{	
	for (int i = 0; i < current_dim; i++ ) 
		if (dimState[i] == ON) 
		{
			zeroCross[i] = 1;
		}
}

static int k;
void onTimerISR()
{
	digitalWrite(PC13, !(digitalRead(PC13)));
	toggleCounter++;
	for (k = 0; k < current_dim; k++)
	{
		
		if (zeroCross[k] == 1 )
		{
			
			dimCounter[k]++;

			if (dimMode[k] == TOGGLE_MODE)
			{
			/*****
			 * TOGGLE DIMMING MODE
			 *****/
			if (dimPulseBegin[k] >= togMax[k]) 	
			{
				// if reach max dimming value 
				togDir[k] = false;	// downcount				
			}
			if (dimPulseBegin[k] <= togMin[k])
			{
				// if reach min dimming value 
				togDir[k] = true;	// upcount
			}
			if (toggleCounter == toggleReload) 
			{
				if (togDir[k] == true) dimPulseBegin[k]++;
				else dimPulseBegin[k]--;
			}
			}
			
			/*****
			 * DEFAULT DIMMING MODE (NOT TOGGLE)
			 *****/
			if (dimCounter[k] >= dimPulseBegin[k] )
			{
				digitalWrite(dimOutPin[k], HIGH);	
			}

			if (dimCounter[k] >=  (dimPulseBegin[k] + pulseWidth) )
			{
				digitalWrite(dimOutPin[k], LOW);
				zeroCross[k] = 0;
				dimCounter[k] = 0;
			}
		}
	}
	if (toggleCounter >= toggleReload) toggleCounter = 1;
}

#endif
