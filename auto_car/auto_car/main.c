/*
 * 04.timer_test.c
 *
 * Created: 2024-04-26 오후 3:23:30
 * Author : kccistc
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>   // printf scanf fgets 등이 정의 되어 있다. 

#include "def.h"
#include "extern.h"
#include "ultrasonic.h"

void init_timer0();
void init_uart0(void);
void init_pwm_motor(void);
void forward(int speed);
void backward(int speed);
void left(int speed);
void right(int speed);
void stop();
void auto_drive();

extern void init_led();
extern void auto_mode_check();

// 1. for printf
FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);


volatile uint32_t ms_count=0;   // ms count
                                // interrupt service routine 안에 쓰는 변수는
							    // 변수 type앞에 volatile이라고 기입 한다.
								// 이유는 최적화 방지를 위함이다. 
volatile uint32_t shift_timer=0; 
volatile uint32_t ultrasonic_trigger_timer=0;	// trigger를 동작시키는 주기 1000ms
// 256개의 pulse를 count(1ms)하면 이곳으로 자동적으로 진입 한다.
// 즉 256개의 pulse ==> 1ms
// ISR 루틴은 가능한 짧게 구현 한다. 
ISR(TIMER0_OVF_vect)
{
	TCNT0=6;  // 6~256 --> 1ms를 맞춰주기 위해서 TCNT0를 6으로 설정
	ms_count++;
	shift_timer++;
	ultrasonic_trigger_timer++;
}

extern int button1_state;
int main(void)
{	
	init_led();
	init_timer0();
	init_uart0();
	init_uart1();
	init_button();
	init_pwm_motor();
	stdout = &OUTPUT;   // 2. printf가 동작 되도록 stdout에 OUTPUT화일 포인터를 assign
	                    // fprintf(stdout,"test\n");  printf("test\n"); scanf("%s"buff);  ==> stdin
	                    // fgets(input,100,stdin); // stdin : keyboard
	
	init_ultrasonic();
	// I2C_LCD_init();
	
	sei();    // 전역적으로 인터럽트 허용
	
	
	
//I2C_LCD_Test();
//make_pwm_led_control();


    while (1) 
    {
		manual_mode_run();	// bt command로 제어
		auto_mode_check();
		if (button1_state)	// button1이 on
		{
			auto_drive();
		}
		// ultrasonic_distance_check();
    }
}

void auto_drive()
{
	
}


// timer0를 초기화 한다. 8bit timer 0번과 2번중 0번을 사용한다. 
void init_timer0()
{
// 분주비를 64로 설정 
// 1.분주비 계산
// 16000000HZ/64 ==> 250,000 HZ
// 2. T(주기) pulse 1개가 잡아 먹는 시간: 1/f = 1/250000 => 0.000004sec (4us) : 0.004ms
// 3. 8bit timer OV(OV flow) : 0.004ms x 256 == 0.001024sec ==> 1.024ms
//                             0.004ms x 250 == 0.001sec ==> 1ms
  TCNT0 = 6;   // TCNT : 0~256 ==> 정확히 1ms마다 TIMER0_OVF_vect로 진입한다.
  TCCR0 |= (1 << CS02) | (0 << CS01) | (0 << CS00);  // (1) 분주비: 64로 설정
  TIMSK |= 1 << TOIE0;    // (2) timer0 overflow INT허용
}


// 초기화 timer1
/*
	1. LEFT MOTOR
		PF0 : IN1
		PF1 : IN2
	2. RIGHT MOTOR
		PF2 : IN3
		PF3 : IN4
		
		IN1 IN2
		=== ===
		 1	 0 --> 정회전
		 0	 1 --> 역회전
		 1	 1 --> STOP
*/ 

#define MOTOR_DRIVER_PORT	PORTF
#define MOTOR_DRIVER_PORT_DDR DDRF

#define MOTOR_DDR DDRB
#define MOTOR_RIGHT_PORT_DDR 5	// OC1A
#define MOTOR_LEFT_PORT_DDR 6	// OC1B
void init_pwm_motor(void)
{
	MOTOR_DRIVER_PORT_DDR |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;	// IN1 IN2 IN3 IN4 --> 출력모드 set
	MOTOR_DDR |= 1 << MOTOR_RIGHT_PORT_DDR | 1 << MOTOR_LEFT_PORT_DDR;	// pwm port
	// 16000000Hz / 64 --> 250000Hz (timer1에 공급되는 clock)
	// pulse를 256개를 count한다 : 256 / 250000 --> 1.02ms
	// pulse의 절반 127개를 count: 127 / 250000 --> 0.5ms
	// 0x3ff(1023) --> 4ms
	TCCR1B |= 1 << CS11 | 1 << CS10;	// 64분주
	
	TCCR1B |= 1 << WGM13 | 1 << WGM12;	// 모드14 고속PWM ICR1
	TCCR1A |= 1 << WGM11 | 0 << WGM10;	// 모드14
	
	TCCR1A |= 1 << COM1A1 | 0 << COM1A0;	// 비반전 모드: OCR값을 만나면 PWM신호가 LOW가 되고
											//			   TOP값을 만나면 PWM신호가 HIGH가 된다
	TCCR1A |= 1 << COM1B1 | 0 << COM1B0;	// 비반전 모드
	
	MOTOR_DRIVER_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);	// all clear
	MOTOR_DRIVER_PORT |= 1 <<2 | 1 << 0;	// 정회전 모드 set
	ICR1 = 0x3ff;	// 0x3ff(1023) : 4ms	
}

// 'F' 전진
// 'B' 후진
// 'L' 좌회전
// 'R' 우회전
extern volatile uint8_t uart1_rx_data;
void manual_mode_run(void)
{
	switch(uart1_rx_data)
	{
		case 'F':	// forward
		forward(500);	// 4us x 500 --> 0.002sec(2ms)
		break;
		
		case 'B':	// backward
		backward(500);
		break;
		
		case 'L':	// left
		left(700);
		break;
		
		case 'R':	// right
		right(700);
		break;
		
		case 'S':	// right
		stop();
		break;
		
		default:
		break;
	}
}

void forward(int speed)
{
	MOTOR_DRIVER_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);	// all clear
	MOTOR_DRIVER_PORT |= 1 <<2 | 1 << 0;	// 정회전 모드 set
	
	OCR1A =	speed;	// PB.5 PWM 출력 left
	OCR1B =	speed;	// PB.6 PWM 출력 right
}

void backward(int speed)
{
	MOTOR_DRIVER_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);	// all clear
	MOTOR_DRIVER_PORT |= 1 << 1 | 1 << 3;	// 0101 역회전 모드 set
	
	OCR1A =	speed;	// PB.5 PWM 출력 left
	OCR1B =	speed;	// PB.6 PWM 출력 right
}

void left(int speed)
{
	MOTOR_DRIVER_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);	// all clear
	MOTOR_DRIVER_PORT |= 1 <<2 | 1 << 0;	// 정회전 모드 set
	
	OCR1A =	0;	// PB.5 PWM 출력 left
	OCR1B =	speed;	// PB.6 PWM 출력 right
}

void right(int speed)
{
	MOTOR_DRIVER_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);	// all clear
	MOTOR_DRIVER_PORT |= 1 <<2 | 1 << 0;	// 정회전 모드 set
	
	OCR1A =	speed;	// PB.5 PWM 출력 left
	OCR1B =	0;	// PB.6 PWM 출력 right
}

void stop()
{
	MOTOR_DRIVER_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);	// all clear
	MOTOR_DRIVER_PORT |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;	// stop 모드 set
	
	OCR1A =	0;	// PB.5 PWM 출력 left
	OCR1B =	0;	// PB.6 PWM 출력 right
}