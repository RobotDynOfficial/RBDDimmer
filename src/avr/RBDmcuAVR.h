#ifndef RBDMCUAVR_H
#define RBDMCUAVR_H

#include "Arduino.h"
#include "RBDdimmer.h"
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define DIMMER_TIMER 4
#define INT_vect INT4_vect   
#define INTx INT4
#define EICRX EICRB
#define ISCx1 ISC41
#define ISCx0 ISC40
#define ALL_DIMMERS 50

#define TCCRxA_VALUE 0x00 // CTC mode
#define TCCRxB_VALUE 0x0A // 0b1011 // (1 << WGMx2)|(1 << CSx1)|(1 << CSx0)
#define OCRxAH_VALUE 0x00
#define OCRxAL_VALUE 0x0F

#define _OCRxAH(X) OCR ## X ## AH
#define OCRxAH(X) _OCRxAH(X)
#define _OCRxAL(X) OCR ## X ## AL
#define OCRxAL(X) _OCRxAL(X)

#elif defined(__AVR_ATmega32U4__)
#define DIMMER_TIMER 1
#define INT_vect INT6_vect
#define INTx INT6
#define EICRX EICRB
#define ISCx1 ISC61
#define ISCx0 ISC60
#define ALL_DIMMERS 30

#define TCCRxA_VALUE 0x00 // CTC mode
#define TCCRxB_VALUE 0x09// 0b1011 (1 << WGMx2)|(1 << CSx1)|(1 << CSx0)
#define OCRxAH_VALUE 0x00
#define OCRxAL_VALUE 0xBC

#define _OCRxAH(X) OCR ## X ## AH
#define OCRxAH(X) _OCRxAH(X)
#define _OCRxAL(X) OCR ## X ## AL
#define OCRxAL(X) _OCRxAL(X)

#else
#define DIMMER_TIMER 2
#define INT_vect INT0_vect
#define INTx INT0
#define EICRX EICRA
#define ISCx1 ISC01
#define ISCx0 ISC00
#define ALL_DIMMERS 13

#define TCCRxA_VALUE 0x02
#define TCCRxB_VALUE 0x09 // 0b1010 // (1 << WGMx2)|(1 << CSx1)
#define OCRxAH_VALUE 0x00
#define OCRxAL_VALUE 0xFF

#define _OCRxAH(X) OCR ## X ## A
#define OCRxAH(X) _OCRxAH(X)
#define _OCRxAL(X) OCR ## X ## A
#define OCRxAL(X) _OCRxAL(X)

#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__)
static const uint16_t powerBuf[] = {
     610,   604, 598,  592,  586,  580,  574,  568,  562,   556,
     550,   544, 538,  532,  526,  520,  514,  508,  502,   496,
     490,   484, 478,  472,  466,  460,  454,  448,  442,   436,
     430,   424, 418,  412,  406,  400,  394,  388,  382,   376,
     370,   364, 358,  352,  346,  340,  334,  328,  322,   316,
     310,   304, 298,  292,  286,  280,  274,  268,  262,   256,
     250,   244, 238,  232,  226,  220,  214,  208,  202,   196,
     180,   174, 168,  162,  156,  150,  144,  138,  132,   126,
     120,   114, 108,  102,  96,   90,   84,   78,   72,     66,
      60,    54,  48,   42,  36,   30,   24,   18,   12,      6
};
#else
static const uint16_t powerBuf[] = {
     600,   600, 598,  592,  586,  580,  574,  568,  562,   556,
     550,   544, 538,  532,  526,  520,  514,  508,  502,   496,
     490,   484, 478,  472,  466,  460,  454,  448,  442,   436,
     430,   424, 418,  412,  406,  400,  394,  388,  382,   376,
     370,   364, 358,  352,  346,  340,  334,  328,  322,   316,
     310,   304, 298,  292,  286,  280,  274,  268,  262,   256,
     250,   244, 238,  232,  226,  220,  214,  208,  202,   196,
     180,   174, 168,  162,  156,  150,  144,  138,  132,   126,
     120,   114, 108,  102,  96,   90,   84,   78,   72,     66,
      60,    54,  48,   42,  36,   30,   24,   18,   12,      8
};
#endif

#endif
