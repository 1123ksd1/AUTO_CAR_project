/*
 * ultrasonic.h
 *
 * Created: 2024-05-08 오전 10:55:22
 *  Author: HARMAN-04
 */ 


#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#define F_CPU 16000000UL
#include <avr/io.h>   
#include <util/delay.h>   
#include <avr/interrupt.h>

#define TRIG_DDR DDRG
#define TRIG_PORT PORTG
#define TRIG 4

#define ECHO_DDR DDRE
#define ECHO_PIN PINE
#define ECHO 4

void init_ultrasonic();
void ultrasonic_trigger();
void ultrasonic_distance_check();
void light_on();


#endif /* ULTRASONIC_H_ */