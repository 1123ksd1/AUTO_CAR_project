/*
 * button.h
 *
 * Created: 2024-04-25 오후 1:15:17
 *  Author: kccistc
 */ 


#ifndef BUTTON_H_
#define BUTTON_H_
#define F_CPU 16000000UL
#include <avr/io.h>   // PORTA DDRA 등의 symbol이 정의 되어 있다.'
#include <util/delay.h>   // _delay_ms 등의 함수가 들어 있다. 

#define LED_DDR  DDRA   // 이렇게 하는 이유는 LED_DDR에서 바꿔주면 compiler가 알아서 변경
#define LED_PORT PORTA 
#define BUTTON_DDR  DDRC
#define BUTTON_PIN PINC   // PORTC를 read하는 register 전압5v를 읽으면 1, 0v를 읽으면 0

#define BUTTON1PIN   7   // PORTD.7 

#define BUTTON1   0    


#define BUTTON_PRESS 1   // 버튼을 누르면 high  active-high
#define BUTTON_RELEASE 0  // 버튼을 떼면 low 
#define BUTTON_NUMBER 4   // 버튼 갯수

#endif /* BUTTON_H_ */