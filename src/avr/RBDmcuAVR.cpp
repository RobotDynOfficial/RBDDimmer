#if defined(ARDUINO_ARCH_AVR)

#include "RBDmcuAVR.h"

#define _TCNT(X) TCNT ## X
#define TCNT(X) _TCNT(X)
#define _TCCRxA(X) TCCR ## X ## A
#define TCCRxA(X) _TCCRxA(X)
#define _TCCRxB(X) TCCR ## X ## B
#define TCCRxB(X) _TCCRxB(X)
#define _TIMSKx(X) TIMSK ## X
#define TIMSKx(X) _TIMSKx(X)
#define _TIFRx(X) TIFR ## X
#define TIFRx(X) _TIFRx(X)
#define _OCFxA(X) OCF ## X ## A
#define OCFxA(X) _OCFxA(X)
#define _OCFxB(X) OCF ## X ## B
#define OCFxB(X) _OCFxB(X)
#define _TOIEx(X) TOIE ## X
#define TOIEx(X) _TOIEx(X)
#define _OCIExA(X) OCIE ## X ## A
#define OCIExA(X) _OCIExA(X)
#define _TIMER_COMPA_VECTOR(X) TIMER ## X ## _COMPA_vect
#define TIMER_COMPA_VECTOR(X) _TIMER_COMPA_VECTOR(X)

int pulseWidth = 1;
volatile int current_dim = 0;
int all_dim = 3;
int rise_fall = true;
char user_zero_cross = '0';

static int toggleCounter = 0;
static int toggleReload = 25;

static dimmerLamp* dimmer[ALL_DIMMERS];
volatile uint16_t dimPower[ALL_DIMMERS];
volatile uint16_t dimOutPin[ALL_DIMMERS];
volatile uint16_t zeroCross[ALL_DIMMERS];
volatile DIMMER_MODE_typedef dimMode[ALL_DIMMERS];
volatile ON_OFF_typedef dimState[ALL_DIMMERS];
volatile uint16_t dimCounter[ALL_DIMMERS];
static uint16_t dimPulseBegin[ALL_DIMMERS];
volatile uint16_t togMax[ALL_DIMMERS];
volatile uint16_t togMin[ALL_DIMMERS];
volatile bool togDir[ALL_DIMMERS];

dimmerLamp::dimmerLamp(int user_dimmer_pin):
	dimmer_pin(user_dimmer_pin)
{
	current_dim++;
	dimmer[current_dim-1] = this;
	current_num = current_dim-1;
	timer_num = DIMMER_TIMER;
	toggle_state = false;
	
	dimPulseBegin[current_dim-1] = 1;
	dimOutPin[current_dim-1] = user_dimmer_pin;
	dimCounter[current_dim-1] = 0;
	zeroCross[current_dim-1] = 0;
	dimMode[current_dim-1] = NORMAL_MODE;
	togMin[current_dim-1] = 0;
	togMax[current_dim-1] = 1;
	pinMode(user_dimmer_pin, OUTPUT);
}
 
void dimmerLamp::timer_init(void)
{
	TCCRxA(DIMMER_TIMER) &= ~(0xFF); // clean TCCRxA register
	TCCRxB(DIMMER_TIMER) &= ~(0xFF); // clean TCCRxB register

	TIMSKx(DIMMER_TIMER) |= (1 << OCIExA(DIMMER_TIMER)); //устанавливаем бит разрешения прерывания 1ого счетчика по совпадению с OCR1A(H и L)

	TCCRxB(DIMMER_TIMER) = TCCRxB_VALUE;
	if (DIMMER_TIMER != 2) OCRxAH(DIMMER_TIMER) = OCRxAH_VALUE;
	OCRxAL(DIMMER_TIMER) = OCRxAL_VALUE;

	TIMSKx(DIMMER_TIMER) |= (1 << TOIEx(DIMMER_TIMER)); //timer interrupt enable
}

void dimmerLamp::ext_int_init(void) 
{ 
	EICRX &= ~0xFF; 

	EIMSK |= (1 << INTx); 
	EICRX |= (1 << ISCx1)|(1 << ISCx0);//0b00001100 
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
	togMin[this->current_num] = minValue;
	togMax[this->current_num] = maxValue;

	toggleReload = 50;
}

ISR(INT_vect)
{
	for (int i = 0; i < current_dim; i++ ) 
		if (dimState[i] == ON) 
		{
			zeroCross[i] = 1;
		}
}

static int k;
ISR (TIMER_COMPA_VECTOR(DIMMER_TIMER))
{	
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
	TIFRx(DIMMER_TIMER) |= ((1<<OCFxB(DIMMER_TIMER))|(1<<OCFxA(DIMMER_TIMER)));	
}

#endif