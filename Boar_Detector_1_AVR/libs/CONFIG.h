/*
 * PIN_CONFIG.h
 *
 * Created: 2017.03.20 21:04:50
 *  Author: Kieran
 */ 




#ifndef CONFIG_H_
#define CONFIG_H_

// To Check ring pin if modem is turned on
#define RING_INT1_PIN PD3
#define RING_INT1_PORT_R PORTD
#define RING_INT1_PIN_R PIND

extern void pin_cfg();
extern void timer0_init();
extern void int_init();
extern void Vibration_detect_int_off();
extern void Vibration_detect_int_on();
extern void standart_init();
extern void Ring_detection_int_on();
extern void Ring_detection_int_off();

#endif /* PIN_CONFIG_H_ */