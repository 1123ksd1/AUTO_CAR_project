/*
 * ultrasonic.c
 *
 * Created: 2024-05-08 오전 10:55:38
 *  Author: HARMAN-04
 */ 
#include <stdio.h>
#include "ultrasonic.h"
#include "extern.h"
volatile int ultrasonic_distance = 0;
char scm[50] = {0,};

// PE4: 외부 INT4 초음파세넛 상승, 하강에지 둘 다 이곳으로 들어온다.
// 결국은 상승에지에서 1번/ 하강에지에서 1번 2번 들어온다.
ISR(INT4_vect)
{
	// 1. 상승에지
	if(ECHO_PIN & (1 << ECHO))
	{
		TCNT1 = 0;
	}
	// 2. 하강에지
	else
	{
		// 에코핀에 들어오는 펄스 길이를 us 단위로 환산
		ultrasonic_distance = 1000000.0 * TCNT1 * 1024 / F_CPU;
		// ex) TCNT1에 10이 들어 있다
		// 15.625KHz의 1주기가 64us
		// 0.000064 * 10 = 0.00064sec(640us)
		// 640 / 58 --> 11cm
		// 1cm --> 58us소요  
		sprintf(scm, "dis: %d cm", ultrasonic_distance/58);	// cm로 환산
	}
}

void init_ultrasonic()	// InitUltrasonic -> microsoft 방식
{
	TRIG_DDR |= 1<<TRIG;	// Output mode 설정
	ECHO_DDR |= ~(1<<ECHO); // Input mode 설정
	
	// P289 표12-6
	// 0 1 : 상승 하강 에지 둘 다 interrupt 를 띄우도록 설정한다.
	EICRB |= 0 << ISC41 | 1 << ISC40;
	// 16bit timer 1번을 설정해서 사용한다.
	// 16 65535
	// 16MHz : clock
	// 16MHz를 1024분주 16000000/1024 --> 15625 --> 15.625KHz
	// 1주기(1개의 펄스 길이) T = 1/f 1/15625 -> 0.000064sec --> 64us
	
	TCCR3B |= 1 << CS32 | 1 << CS30;	// 1024로 분주 설정
	
	EIMSK |= 1 << INT4;	// 외부 INT4(ECHO) 사용
}

void ultrasonic_trigger()
{
	TRIG_PORT &= ~(1 << TRIG);	// low
	_delay_us(1);
	TRIG_PORT |= 1 << TRIG;		// high
	_delay_us(15);
	TRIG_PORT &= ~(1 << TRIG);	// low
}

void ultrasonic_distance_check()
{
	if(ultrasonic_trigger_timer > 1000)		// 1sec
	{
		ultrasonic_trigger_timer = 0;
		printf("%s", scm);
		ultrasonic_trigger();
	}	
}

void light_on()
{	
	if(ultrasonic_distance/58 < 1)
	{
		PORTA = 0b00000001;
	}
	else if(1 <= ultrasonic_distance/58 && ultrasonic_distance/58 < 2)
	{
		PORTA = 0b00000011;
	}
	else if(2 <= ultrasonic_distance/58 && ultrasonic_distance/58 < 3)
	{
		PORTA = 0b00000111;
	}
	else if(3 <= ultrasonic_distance/58 && ultrasonic_distance/58 < 4)
	{
		PORTA = 0b00001111;
	}
	else if(5 <= ultrasonic_distance/58 && ultrasonic_distance/58 < 6)
	{
		PORTA = 0b00011111;
	}
	else
		PORTA = 0b00000000;
		
}