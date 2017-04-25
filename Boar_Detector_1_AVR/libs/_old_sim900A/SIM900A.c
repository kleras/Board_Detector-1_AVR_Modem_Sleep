/*
 * SIM900A.c
 *
 * Created: 2016.08.13 19:07:03
 *  Author: Kieran
 */ 

#include <util/delay.h>
#include <avr/wdt.h>


void modem_init()
{
	
}


void majak(char number)
{
			dbg_uart_puts("Starting to call!\r\n");
			_delay_ms(500);		
			
			
			dbg_uart_puts("ATD+37061217788;\r\n");
			_delay_ms(6500); //6750
			dbg_uart_puts("ATH\r\n");
			
			_delay_ms(2500);
}


void send_sms(char number, char sms_text)
{
		wdt_reset();
	
		dbg_uart_puts("AT+CMGS=");
		uart_putc(0x22);
		dbg_uart_puts("+37061217788");
		uart_putc(0x22);
		dbg_uart_puts("\r\n");
		
		_delay_ms(10);
		
		wdt_reset();
		
		dbg_uart_puts(sms_text);
		uart_putc(0x1A);
		dbg_uart_puts("\r\n");
		
		wdt_reset();
		
		_delay_ms(100);
		
		wdt_reset();
}