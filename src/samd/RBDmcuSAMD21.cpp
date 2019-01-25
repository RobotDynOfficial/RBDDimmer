#if defined(ARDUINO_ARCH_SAMD)

#include "RBDmcuSAMD21.h"

int dim_tim[10];
int dim_max[10];

//int dim_begin = 90;
int pulseWidth = 2;
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
	toggle_state = false;
	
	dimPulseBegin[current_dim-1] = 1;
	dimOutPin[current_dim-1] = user_dimmer_pin;
	dimCounter[current_dim-1] = 0;
	zeroCross[current_dim-1] = 0;
	dimMode[current_dim-1] = NORMAL_MODE;
	togMin[current_dim-1] = 0;
	togMax[current_dim-1] = 1;
	pinMode(user_dimmer_pin, OUTPUT);
	__enable_irq();
}
 
void dimmerLamp::timer_init(void)
{
	
	SYSCTRL->OSC8M.bit.PRESC = 0;                          // no prescaler (is 8 on reset)
	SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_ENABLE;   // enable source

	GCLK->GENDIV.bit.ID = 0x07;                            // select GCLK_GEN[1]
	GCLK->GENDIV.bit.DIV = 0;                              // no prescaler

	GCLK->GENCTRL.bit.SRC = 0x06;
	GCLK->GENCTRL.bit.ID = 0x07;                           // select GCLK_GEN[1]
	GCLK->GENCTRL.bit.GENEN = 1;                           // enable generator

	PM->APBCSEL.bit.APBCDIV = 0;			// no prescaler
	PM->APBCMASK.bit.TC3_ = 1;				// enable TC3 interface
	
	// Configure asynchronous clock source
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TCC2_TC3;   // select TC3 peripheral channel
	GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_GEN_GCLK7;		// select source GCLK_GEN[0]
	GCLK->CLKCTRL.bit.CLKEN = 1;						// enable TC3 generic clock
	
	// Configure synchronous bus clock
	TC3->COUNT8.CTRLA.bit.MODE = 0x1;		// Configure Count Mode (16-bit)
	TC3->COUNT8.CTRLA.bit.WAVEGEN = 0x01;
	TC3->COUNT8.CTRLA.bit.PRESCALER = 0x02;	// Configure Prescaler for divide by 2 (500kHz clock to COUNT)
	TC3->COUNT8.CTRLBCLR.bit.DIR = 1;
	TC3->COUNT8.CTRLC.bit.CPTEN1 = 1;
	TC3->COUNT8.INTENSET.bit.MC0 = 1;	// Enable TC3 compare mode interrupt generation // Enable match interrupts on compare channel 0
	TC3->COUNT8.CC[0].reg = 190;			// Initialize compare value for 100mS @ 500kHz
	
	while(TC3->COUNT8.STATUS.bit.SYNCBUSY == 1);	// Wait until TC3 is enabled
	TC3->COUNT8.CTRLA.bit.ENABLE = 1;		// Enable TC3
	NVIC_EnableIRQ(TC3_IRQn);			// Enable TC3 NVIC Interrupt Line
}

void dimmerLamp::ext_int_init(void) 
{ 
	// Configure interrupt port (D8)
	PORT->Group[PORTA].WRCONFIG.reg |=
	PORT_WRCONFIG_WRPINCFG	|										// Enables the configuration of PINCFG
	PORT_WRCONFIG_WRPMUX	|										// Enables the configuration of the PMUX for the selected pins
	PORT_WRCONFIG_PMUX(MUX_PA15A_EIC_EXTINT15) |						// Bulk configuration for PMUX EIC for PA06
	PORT_WRCONFIG_PMUXEN	|										// Enables the PMUX for the pins
	PORT_WRCONFIG_PINMASK(PORT_PA15)|
	PORT_WRCONFIG_PULLEN	|
	PORT_WRCONFIG_INEN;												// Enable input
	
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_CLKEN;
	REG_PM_APBCMASK |= PM_APBAMASK_EIC;
	
	while (EIC->CTRL.bit.SWRST | EIC->STATUS.bit.SYNCBUSY);
	EIC->CTRL.reg |= EIC_CTRL_SWRST;
	EIC->CONFIG[1].reg |= EIC_CONFIG_FILTEN7 | EIC_CONFIG_SENSE7_RISE;
	EIC->INTENSET.reg |= EIC_INTENSET_EXTINT15;
	EIC->CTRL.reg |= EIC_CTRL_ENABLE;	// Enable EIC
	
	NVIC_EnableIRQ(EIC_IRQn);
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

void EIC_Handler(void)
{
	for (int i = 0; i < current_dim; i++ ) 
		if (dimState[i] == ON) 
		{
			zeroCross[i] = 1;
		}
	EIC->INTFLAG.reg |= EIC_INTFLAG_EXTINT15;	// clear interrupt flag
}


static int k;
void TC3_Handler(void)
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
	TC3->COUNT8.INTFLAG.reg = TC_INTFLAG_MC0;	// Acknowledge the interrupt (clear MC0 interrupt flag to re-arm)
}

#endif
