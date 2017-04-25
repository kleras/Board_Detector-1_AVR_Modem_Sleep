/*
 * PIN_CONFIG.c
 *
 * Created: 2017.03.20 21:05:25
 *  Author: Kieran
 */ 


#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>


//*** MISQ ****
#define off(a,z) (a &= ~_BV(z))
#define on(a,z) (a |= _BV(z))
#define change(a,z) (a ^=_BV(z))
#define bit_is_clear(sfr,bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define bit_is_set(sfr, bit)   (_SFR_BYTE(sfr) & _BV(bit))


// ***** INPUTs/OUTPUTs ******

#define ADC_VBAT_PIN PC3

#define DIN1_PIN PD4
#define DIN1_PIN_R PIND // Needs pullup //DIN1_PORT_R |= (1<<PD4);
#define DIN1_PORT_R PORTD

//on(DIN1_PORT_R,DIN1_PIN);

#define VIBR_INT0_PIN PD2
#define VIBR_INT0_PORT_R PORTD // Needs pullup //DIN1_PORT_R |= (1<<PD4);

//on(VIBR_INT0_PORT_R,VIBR_INT0_PIN);

#define RING_INT1_PIN PD3
#define RING_INT1_PORT_R PORTD
#define RING_INT1_PIN_R PIND

//on(RING_INT1_PORT_R,RING_INT1);


	
void pin_cfg()

{
	
	//The DDxn bit in the DDRx Register selects the direction of this pin. If DDxn is written logic one, Pxn is
	//configured as an output pin. If DDxn is written logic zero, Pxn is configured as an input pin.
	//If PORTxn is written logic one when the pin is configured as an input pin, the pull-up resistor is activated.
	
	/*
	When switching between tri-state ({DDxn, PORTxn} = 0b00) and output high ({DDxn, PORTxn} = 0b11), an
	intermediate state with either pull-up enabled {DDxn, PORTxn} = 0b01) or output low ({DDxn, PORTxn} = 0b10)
	must occur.
	*/
	
		
	// Inputs
	
	on(DIN1_PORT_R,DIN1_PIN); // Pull- UP
	
}

void timer0_init()
{
	//Timer 0
	
	TCCR0B |= (1<<CS00) | (1<<CS02);
	TIMSK0 |= (1<<TOIE0);

}

void int_init()
{
	// Interrupts
	
	on(VIBR_INT0_PORT_R,VIBR_INT0_PIN); // INT0 Pull-Up enable. Default - INT0 int. on LOW level.	
	
	on(RING_INT1_PORT_R,RING_INT1_PIN); // INT0 Pull-Up enable.
	on(EICRA, ISC11);     // The falling edge of INT1 generates an interrupt request.
	
	on(EIMSK, INT0); // INT0 external interrupt enable.
	on(EIMSK, INT1); // INT1 external interrupt enable.
}

void Vibration_detect_int_off()
{
	
	off(EIMSK, INT0); //Atmega328
}

void Vibration_detect_int_on()
{
	on(EIMSK, INT0);
}

void standart_init()
{
	
	// Interrupts
	on(VIBR_INT0_PORT_R,VIBR_INT0_PIN); // INT0 Pull-Up enable. Default - INT0 int. on LOW level.		

	
	// Ring pin config.	
	on(RING_INT1_PORT_R,RING_INT1_PIN); // INT0 Pull-Up enable.
	on(EICRA, ISC11);     // The falling edge of INT1 generates an interrupt request.
	on(EICRA, ISC10);
	
	
	timer0_init();	
	
	
}

void Ring_detection_int_on()
{	
	on(EIMSK, INT1); // INT1 external interrupt enable.	
}

void Ring_detection_int_off()
{	
	off(EIMSK, INT1); // INT1 external interrupt enable.	
	#ifdef DEBUG_MODE
	dbg_puts("RING detection off.\r\n");
	#endif
}

