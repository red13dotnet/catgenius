/******************************************************************************/
/* File    :	catgenius.c						      */
/* Function:	CatGenius Application					      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "../common/timer.h"
#include "../common/hardware.h"
#include "../common/serial.h"
#include "../common/i2c.h"
#include "../common/cr14.h"
#include "../common/srix4k.h"
#include "../common/catsensor.h"
#include "../common/catgenie120.h"
#include "userinterface.h"
#include "litterlanguage.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#ifdef __DEBUG
__CONFIG(XT & WDTDIS & PWRTEN & BOREN & LVPDIS & DUNPROT & WRTEN & DEBUGEN  & UNPROTECT);
#else
__CONFIG(XT & WDTEN  & PWRTEN & BOREN & LVPDIS & DPROT   & WRTEN & DEBUGDIS & PROTECT);
#endif


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned char	PORTB_old;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void interrupt_init (void);


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void main (void)
{
	unsigned char	flags;

	/* Init the hardware */
	flags = catgenie_init();

	/* Initialize software timers */
	timer_init();

	/* Initialize the I2C bus */
	i2c_init();

	/* Initialize the RFID reader */
	cr14_init();

	/* Initialize the RFID tag */
	srix4k_init();

	/* Initialize the serial port */
	serial_init();

	/* Initialize the cat sensor */
	catsensor_init();

	/* Initialize the user interface */
	userinterface_init();

	/* Initialize the washing program */
	litterlanguage_init();

	/* Initialize interrupts */
	interrupt_init();

	/* Execute the run loop */
	for(;;){
		catsensor_work();
		catgenie_work();
		userinterface_work();
		litterlanguage_work();
		srix4k_work();
		cr14_work();
		i2c_work();
#ifndef __DEBUG
		CLRWDT();
#endif
//	putch('A');

	}
}

void catsensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	catsensor_event						      */
/*		- Handle state changes of cat sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if (detected)
		set_LED_Cat(0x55, 1);
	else
		set_LED_Cat(0x00, 0);
}
/* catsensor_event */

void watersensor_event (unsigned char undetected)
/******************************************************************************/
/* Function:	watersensor_event					      */
/*		- Handle state changes of water sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* watersensor_event */

void heatsensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	heatsensor_event					      */
/*		- Handle state changes of over-heat sensor		      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* heatsensor_event */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void interrupt_init (void)
{
	PORTB_old = PORTB;

	/* Enable peripheral interrupts */
	PEIE = 1;

	/* Enable interrupts */
	GIE = 1;
}

static void interrupt isr (void)
{
	unsigned char temp;

	/* Timer 1 interrupt */
	if (TMR1IF) {
		/* Reset interrupt */
		TMR1IF = 0;
		/* Handle interrupt */
		timer_isr();
	}
	/* Timer 2 interrupt */
	if (TMR2IF) {
		/* Reset interrupt */
		TMR2IF = 0;
		/* Handle interrupt */
		catsensor_isr_timer();
	}
	/* Port B interrupt */
	if (RBIF) {
		/* Reset interrupt */
		RBIF = 0;
		/* Detected changes */
		temp = PORTB ^ PORTB_old;
		/* Handle interrupt */
		if (temp & CATSENSOR_MASK)
			catsensor_isr_input();
		/* Update the old status */
		PORTB_old = PORTB ;
	}
}