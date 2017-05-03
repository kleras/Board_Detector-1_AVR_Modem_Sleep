/*
 * sim900.c
 *
 * Created: 7/25/2014 8:22:45 AM
 *  Author: EX4
 */ 

#define DEBUG_MODE

#define F_CPU 8000000UL // CPU clock define (CPU should know what frequency is he on)

#define off(a,z) (a &= ~_BV(z))
#define on(a,z) (a |= _BV(z))
#define change(a,z) (a ^=_BV(z))
#define bit_is_clear(sfr,bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define bit_is_set(sfr, bit)   (_SFR_BYTE(sfr) & _BV(bit))

//header file
#include "sim900.h"
#include <Boar_Detector_1_AVR.h>
#include <ADC.h>

void sim900_init_uart(const uint16_t baudrate)
{
	//init uart
	uart_init(UART_BAUD_SELECT(baudrate, F_CPU));
}

void uart_flush_buffer()
{
	while (uart_getc() != UART_NO_DATA);
}

uint8_t sim900_send_cmd_wait_reply(const uint8_t send_from_progmem, const uint8_t *aCmd, const uint8_t
*aResponExit, const uint32_t aTimeoutMax, const uint8_t aLenOut, uint8_t
*aResponOut)
{
	#define MAX_BUFFER_TMP	64

	uint8_t id_data, respons = 0;
	uint32_t uart_tout_cnt = 0;
	uint16_t uart_data;
	
	uint8_t *aDataBuffer = (uint8_t*) calloc(MAX_BUFFER_TMP,sizeof(uint8_t));
	
	//buffer created???
	if (aDataBuffer == NULL)
	{
		return 0;
	}

	//reset to all 0
	memset(aDataBuffer, '\0', MAX_BUFFER_TMP);

	//read left buffer data
	if (aCmd != NULL)
	{
		uart_flush_buffer();
	}

	//send command
	if (aCmd != NULL)
	{
		if (send_from_progmem)
		{
			uart_puts_p((const char*)aCmd);
		}
		else
		{
			uart_puts((const char*)aCmd);
		}
	}

	//wait for reply
	id_data = 0;
	uart_tout_cnt = 0;
	while ((id_data < (MAX_BUFFER_TMP - 1)) && (uart_tout_cnt <= aTimeoutMax))
	{
		//get uart data or timeout
		uart_tout_cnt = 0;
		while (((uart_data = uart_getc()) == UART_NO_DATA) && (uart_tout_cnt <
		aTimeoutMax))
		//wait data arrive or tout
		{
			uart_tout_cnt++;
			_delay_us(1);			
		}

		//check for timeout
		if (uart_tout_cnt >= aTimeoutMax)
		{
			#ifdef DEBUGMODE
			dbg_puts("AT command timeout.\r\n");
			#endif
			
			respons = 0;
			break;
		}
		else
		{
			aDataBuffer[id_data] = uart_data;
			id_data++;

			// check if the desired answer  is in the response of the module
			if (aResponExit != NULL)
			{
				if (strstr((const char*)aDataBuffer, (const char*)aResponExit) != NULL)
				{					
					respons = 1;
					break;
				}
			}

			//check error also
			if (strstr((const char*)aDataBuffer, (const char*)RESPON_ERROR) != NULL)
			{
				respons = 0;
				break;
			}
		}
	}

	//copy it to the out
	if ((aLenOut != 0) && (aResponOut != NULL) && (aLenOut > id_data) && (respons)
	)
	{
		memcpy(aResponOut, aDataBuffer, id_data *sizeof(uint8_t));
	}
	
	//free the buffer
	free(aDataBuffer);

	//return it
	return respons;
}

uint8_t sim900_test_at()
{
	
	return sim900_send_cmd_wait_reply(1,(const uint8_t*)PSTR("AT\r"), (const
	uint8_t*)RESPON_OK, 500000, 0, NULL);	
		
}


uint8_t sim900_is_network_registered()
{
	return (sim900_send_cmd_wait_reply(1,(const uint8_t*)PSTR("AT+CREG?\r"), (const
	uint8_t*)"+CREG: 0,1", 5000000, 0, NULL) || sim900_send_cmd_wait_reply(1,
	(const uint8_t*)PSTR("AT+CREG?\r"), (const uint8_t*)"+CREG: 0,5", 5000000, 0,
	NULL));
}

#warning Reikes sutvarkyt + ided CMEE err reporting.
uint8_t sim900_setup(const uint8_t isWaitingDevice)
{
	uint8_t respons = 0;
	
	#warning Modem setup should include set default config, default settings etc.

	//wait modem wake up
	if (isWaitingDevice)
	{
		#warning AT comman check-up Should be changed to for-cycle.
		while (!(respons = sim900_send_cmd_wait_reply(1,(const uint8_t*)PSTR("AT\r"),
		(const uint8_t*)RESPON_OK, 500000, 0, NULL)))
		;
		_delay_ms(100);
	}
	else
	{
		respons = sim900_send_cmd_wait_reply(1,(const uint8_t*)PSTR("AT\r"), (const
		uint8_t*)RESPON_OK, 500000, 0, NULL);
		_delay_ms(100);	
	
	}
	
		#ifdef DEBUG_MODE
		dbg_puts("Checking config: \r\n");
		#endif
	
	//modem is up?
	if (respons)
	{		
		//no cmd echo
		if ((respons = sim900_send_cmd_wait_reply(1,(const uint8_t*)PSTR("ATE0\r"), (const
		uint8_t*)RESPON_OK, 500000, 0, NULL)))
		{
			_delay_ms(100);
			
			#ifdef DEBUG_MODE
			dbg_puts("[ATE0 ok.]\r\n");
			#endif
			
			//text mode
			if ((respons = sim900_send_cmd_wait_reply(1,(const uint8_t*)PSTR("AT+CMGF=1\r"),
			(const uint8_t*)RESPON_OK, 1000000, 0, NULL)))
			{
				_delay_ms(100);
				
				#ifdef DEBUG_MODE
				dbg_puts("[Text mode set ok.]\r\n");
				#endif
				
				//storage all to Sim card
				// Neveiks jei ner simkes
				if ((respons = sim900_send_cmd_wait_reply(1,(const uint8_t*)
				PSTR("AT+CPMS=\"SM\",\"SM\",\"SM\"\r"), (const uint8_t*)RESPON_OK, 1000000,
				0, NULL)))
				{
					#ifdef DEBUG_MODE
					dbg_puts("[SMS Storage set OK.]\r\n");
					#endif
					_delay_ms(100);
					//clip=1
					if ((respons = sim900_send_cmd_wait_reply(1,(const uint8_t*)
				PSTR("AT+CLIP=1\r"), (const uint8_t*)RESPON_OK, 1000000, 0, NULL)))			
				{					
					#ifdef DEBUG_MODE
					dbg_puts("[Caller ID set OK.]\r\n");
					#endif
					
					_delay_ms(100);
					if ((respons = sim900_send_cmd_wait_reply(1,(const uint8_t*)
					PSTR("AT+CREG=0\r"), (const uint8_t*)RESPON_OK, 1000000, 0, NULL)))
					{
						#ifdef DEBUG_MODE
						dbg_puts("[Registration status set OK.]\r\n");
						#endif
						_delay_ms(100);
						if ((respons = sim900_send_cmd_wait_reply(1,(const uint8_t*)
						PSTR("AT+CMIC=0,15\r"), (const uint8_t*)RESPON_OK, 1000000, 0, NULL)))
						{
							#ifdef DEBUG_MODE
							dbg_puts("[Microphone level set OK.]\r\n");
							#endif
							_delay_ms(100);
							
							if ((respons = sim900_send_cmd_wait_reply(1,(const uint8_t*)
							PSTR("AT+CSCLK=1\r"), (const uint8_t*)RESPON_OK, 1000000, 0, NULL)))
							{
								#ifdef DEBUG_MODE
								dbg_puts("[Sleep mode set ok.]\r\n");
								#endif
								_delay_ms(100);
								if ((respons = sim900_send_cmd_wait_reply(1,(const uint8_t*)
								PSTR("AT+CMGDA=\"DEL ALL\"\r"), (const uint8_t*)RESPON_OK, 5000000,
								0, NULL)))
								{
									#ifdef DEBUG_MODE
									dbg_puts("[Messages deleted.]\r\n");
									#endif
									_delay_ms(100);
								}
								
								
							}
							
						}
					}
				}			
					
			}
		}
	}
}



//return it
return respons;
}

//send sms
uint8_t sim900_send_sms(const uint8_t *aSenderNumber, const uint8_t *aMessage)
{
	const uint8_t MAX_BUFFER = 32;
	uint8_t respon = 0/*, cmdx[MAX_BUFFER]*/;
	uint8_t *cmdx = (uint8_t*) calloc(MAX_BUFFER,sizeof(uint8_t));
	
	//buffer created???
	if (cmdx == NULL)
	{
		return 0;
	}

	//build the string
	memset(cmdx, '\0', MAX_BUFFER); // uzpildo buferi \0 - eil pab. elementais. eol.
	snprintf((char*)cmdx, MAX_BUFFER, "AT+CMGS=\"%s\"\r", aSenderNumber); // \" = "
	
	
		
	if ((respon = sim900_send_cmd_wait_reply(0,(const uint8_t*)cmdx, (const	uint8_t*)">", 2000000, 0, NULL)))
	{
		
		_delay_ms(10);
		
		/*
		//only copy MAX_BUFFER-2 char max from msg
		memcpy(&cmdx[0], aMessage, MAX_BUFFER - 2);
		cmdx[MAX_BUFFER - 2] = 0x1a;
		cmdx[MAX_BUFFER - 1] = 0x00;		
		*/
		
		
		// SMS body.
		/*
		unsigned int f1=0;
		char str[4]="";
		f1 = get_vbat_voltage_mV();
		itoa(f1, str, 10);	
		*/	
		
		uart_puts("Vbat: ");
		uart_puti(get_vbat_voltage_mV());
		uart_puts("mV\r\n");
		
		uart_puts("Temp.: ");
		uart_puti(get_temp_C());
		uart_puts("C\r\n");		
		
		uart_puts("Judesio nebera jau 5min.. Rysys atjungiamas.\r\n");
		
		
		//uart_putc(0x1a);
		
		/*

		if ((respon = sim900_send_cmd_wait_reply(0,(const uint8_t*)cmdx, (const	uint8_t*)RESPON_OK, 10000000, 0, NULL)))	{}
		
		*/
		
		static const uint8_t CTRLZ = 0x1a;
				
		if ((respon = sim900_send_cmd_wait_reply(0, &CTRLZ, (const	uint8_t*)RESPON_OK, 10000000, 0, NULL)))
		{					
			
		}

	}
	

//free buffer
free(cmdx);

return respon;
}


uint8_t sim900_send_sms_template(const uint8_t *aSenderNumber, uint8_t sms_template)
{
	const uint8_t MAX_BUFFER = 32;
	uint8_t respon = 0/*, cmdx[MAX_BUFFER]*/;
	uint8_t *cmdx = (uint8_t*) calloc(MAX_BUFFER,sizeof(uint8_t));
	
	//buffer created???
	if (cmdx == NULL)
	{
		return 0;
	}

	//build the string
	memset(cmdx, '\0', MAX_BUFFER); // uzpildo buferi \0 - eil pab. elementais. eol.
	snprintf((char*)cmdx, MAX_BUFFER, "AT+CMGS=\"%s\"\r", aSenderNumber); // \" = "
	
	
		
	if ((respon = sim900_send_cmd_wait_reply(0,(const uint8_t*)cmdx, (const	uint8_t*)">", 2000000, 0, NULL)))
	{
		
		if(sms_template == 0)
		{
			_delay_ms(10);
			
			uart_puts("Vbat.: ");
			uart_puti(get_vbat_voltage_mV());
			uart_puts("mV");
			
			if(get_vbat_voltage_mV() < BAT_LOW_mV)
			{
				uart_puts("-SENKA.\r\n");
			}
			else
			{
				uart_puts("-OK.\r\n");
			}

			/*
			uart_puts("Temp.: ");
			uart_puti(get_temp_C());
			uart_puts("C\r\n");
			*/
			
			uart_puts("Sistema sujudinta, rysys ijungtas.\r\n");				
			
			uart_putc(0x1a);
		}
		
		if(sms_template == 1) 
		{
			uart_puts("Vbat.: ");
			uart_puti(get_vbat_voltage_mV());
			uart_puts("mV\r\n");
			
			/*
			uart_puts("Temp.: ");
			uart_puti(get_temp_C());
			uart_puts("C\r\n");
			*/
			
			uart_puts("Judesys baigesi. Rysys atjungiamas.\r\n");
			uart_putc(0x1a);
		}
		
		if(sms_template == 2)
		{
			
			uart_puts("Vbat.: ");
			uart_puti(get_vbat_voltage_mV());
			uart_puts("mV\r\n");
			
			/*
			uart_puts("Temp.: ");
			uart_puti(get_temp_C());
			uart_puts("C\r\n");
			*/
			
			uart_puts("Per pagrindini laikotarpi sistema nesujudejo, rekomenduojama pakeisti pirminio filtro nustatymus.\r\n");
			uart_putc(0x1a);
			
		}
		
		/*
		else
		{
			uart_puts("Vbat: ");
			uart_puti(get_vbat_voltage_mV());
			uart_puts("mV\r\n");
			
			
			uart_puts("Program error.\r\n");
			uart_putc(0x1a);			
			
		}			
		*/

	}
	

//free buffer
free(cmdx);

return respon;
}



//calling a phone
uint8_t sim900_call_phone(const uint8_t *aSenderNumber)
{
	const uint8_t MAX_BUFFER = 30;
	uint8_t respon = 0/*, cmdx[MAX_BUFFER]*/;
	uint8_t *cmdx = (uint8_t*) calloc(MAX_BUFFER,sizeof(uint8_t));
	
	//buffer created?
	if (cmdx == NULL)
	{
		return 0;
	}

	//init string
	memset(cmdx, '\0', MAX_BUFFER);
	snprintf((char*)cmdx, MAX_BUFFER, "ATD%s;\r", aSenderNumber);
	respon = sim900_send_cmd_wait_reply(0,(const uint8_t*)cmdx, (const uint8_t*)
	RESPON_OK, 10000000, 0, NULL);
	
	//free buffer
	free(cmdx);
	
	return respon;
}


void sim900_control_pins_init() // intern
{
	// Outputs.
	
	// Make sure output will be LOW.
	
	off(PWRKEY_PORT_R, PWRKEY_PIN); 
	off(DTR_PORT_R, PWRKEY_PIN);
	off(M_RST_PORT_R, PWRKEY_PIN);
	
	on(PWRKEY_DDR_R,PWRKEY_PIN); // OUTPUT, start as LOW
	on(DTR_DDR_R, DTR_PIN);
	on(M_RST_DDR_R,M_RST_PIN);	
	
	on(RING_INT1_PORT_R, RING_INT1_PIN); // Pull-Up on RING.	
	
	_delay_us(10);
	
}


uint8_t sim900_power_on()
{
			
		#warning RING: O jei paskambins arba sms?:D Reikes pakeist kai bus status pin. ir gal void?

		on(PWRKEY_PORT_R, PWRKEY_PIN); // PWRKEY high =  Pull power key to gnd
		_delay_ms(PWRKEY_PULL_MS); // Pull for 1100s
		off(PWRKEY_PORT_R, PWRKEY_PIN); // Release.
		_delay_ms(MODEM_START_MS);

		return 1;	
}

uint8_t sim900_power_off()
{
	/*
	#warning Modem ring again: O jei paskambins arba sms?:D Reikes pakeist kai bus status pin.
	if(bit_is_set(RING_INT1_PIN_R, RING_INT1_PIN)) // O jei paskambins arba sms?:D
	{
		return 1; // Modem already off
	}

	else
	{
		
		*/
		on(PWRKEY_PORT_R, PWRKEY_PIN); // PWRKEY high =  Pull power key to gnd
		_delay_ms(PWRKEY_PULL_MS); // Pull for 1100s
		off(PWRKEY_PORT_R, PWRKEY_PIN); // Release.
		_delay_ms(MODEM_PWROFF_MS);
		
		/*
		
		if(bit_is_set(RING_INT1_PIN_R, RING_INT1_PIN))
		{
			return 1; // Check if ring pin is activated.
		}
		
		else
		{
			return 0;
		}		
	}
	
	*/
		return 1;
}

uint8_t sim900_call_answer()
{		
	return (sim900_send_cmd_wait_reply(1,(const uint8_t*)PSTR("ATA\r"), (const uint8_t*)RESPON_OK, 500000, 0, NULL)); // Dar gali buti ir NO CARRIER		
}

void sim900_sleep_enable()
{
	// Sleep enable DTR high
	on(DTR_PORT_R, DTR_PIN);
	_delay_ms(100);
	#ifdef DEBUG_MODE
	dbg_puts("Modem Sleep enabled.\r\n");
	#endif	
}

void sim900_sleep_disable()
{
	// Sleep disable DTR Low
	off(DTR_PORT_R, DTR_PIN);	
	_delay_ms(200);
	#ifdef DEBUG_MODE
	dbg_puts("Modem Sleep disabled.\r\n");
	#endif
}

