/*
 * led.c
 *
 * Created: 2024-04-29 오전 10:57:55
 *  Author: kccistc
 */ 
#include "led.h"
#include "extern.h"
#include "def.h"

void shift_left2right_keep_ledon(int *pjob);
void shift_right2left_keep_ledon(int *pjob);
void flower_on(int *pjob);
void flower_off(int *pjob);
void make_pwm_led_control(void);
void init_led();
void led_all_on_off();


void init_led()
{
	DDRG |= 1 << 3;
}

void led_all_on_off()
{
	static int led_toggle=0;
		
	if (ms_count >= 300)
	{
		ms_count=0;
		led_toggle = !led_toggle;
		if (led_toggle)
		PORTA=0xff;
		else PORTA=0x00;
	}
}

// 
void shift_left2right_keep_ledon(int *pjob)   // left_to_right
{
#if 1
// 
    static int i=0;
	
	if (shift_timer >= 300)   // 00000001 --> 00000011 .....-> 11111111
	{
		shift_timer=0;
		if (i >= 8)
		{
			i=0;
			PORTA = 0x00;
			*pjob = RIGHT2LEFT;   // RIGHT2LEFT로 상태천이 
		}
		else 
		{
			PORTA |= 0b00000001 << i++;		// 1) PORTA |= 0b00000001 << i 2) i++	
		} 
	} 
#else
	for (int i=0; i < 8; i++)   // 문제점: for문을 2400ms동안 실행 하고 빠져 나온다.
	                            // 2400ms동안 다른 프로그램을 실행 하지 못한다. 
	{
		PORTA |= 0b00000001 << i;
		_delay_ms(300);
	}
	PORTA=0x00;
	_delay_ms(300);
#endif 
}
void shift_right2left_keep_ledon(int *pjob)
{
#if 1
    static int i=0;
    
    if (shift_timer >= 300)   // 00000001 --> 00000011 .....-> 11111111
    {
	    shift_timer=0;
	    if (i >= 8)
	    {
		    i=0;
		    PORTA = 0x00;
			*pjob = FLOWER_ON;    // LEFT2RIGHT로 상태천이 
	    }
	    else
	    {
		    PORTA |= 0b10000000 >> i++;		// 1) PORTA |= 0b10000000 >> i 2) i++
	    }
    }
#else
	for (int i=0; i < 8; i++)
	{
		PORTA |= 0b10000000 >> i;
		_delay_ms(300);
	}
	PORTA=0x00;
	_delay_ms(300);
#endif 
}

void flower_on(int *pjob)
{
#if 1
	static uint8_t  bit_value=0b00010000;
	static int i=0;
	
    if (shift_timer >= 300)   // 00000001 --> 00000011 .....-> 11111111
    {
	    shift_timer=0;
	    if (i >=4 )
	    {
		    i=0;
		    PORTA = 0x00;
		    *pjob = FLOWER_OFF;    // LEFT2RIGHT로 상태천이
	    }
	    else
	    {
			bit_value = bit_value << i | bit_value >> i+1;
			PORTA = bit_value;
			i++;
	    }
    }
#else
	uint8_t  bit_value=0b00010000;
	
	for(int i=0; i < 4; i++)
	{
		PORTA |= bit_value << i | bit_value >> i+1;  
		_delay_ms(30);
	}
	PORTA=0x00;
	_delay_ms(30);
#endif 
}

void flower_off(int *pjob)
{
	static uint8_t  bit_value=0b01111111;
	static i=-1; 

    if (shift_timer >= 300)   // 00000001 --> 00000011 .....-> 11111111
    {
	    shift_timer=0;
		if (i == -1)
		{
			PORTA = 0xff;
			i++;
		}
		else if (i >=4 )
	    {
		    i=-1;
		    PORTA = 0x00;
		    *pjob = LEFT2RIGHT;    // LEFT2RIGHT로 상태천이
	    }
	    else
	    {
			bit_value = bit_value >> i & bit_value << i+1;
		   	PORTA &= bit_value;
			i++;
	    }
    }
		
#if 0 
	PORTA = 0xff;
	_delay_ms(30);
	
	for(int i=0; i < 4; i++)
	{
		PORTA &= bit_value >> i & bit_value << i+1;
		_delay_ms(30);
	}
	PORTA=0x00;
	_delay_ms(30);	
#endif 
}

void make_pwm_led_control(void)
{
	int dim = 0;	// LED밝기 조절 변수
	int direction = 1;	// 1: 밝기 증가 모드 -1: 감소
	
	DDRA = 0xff;
	PORTA = 0xff;	// led all on
	
	//
	while(1)
	{
		led_on_pwm(dim);
		dim += direction;
		if(dim == 255) direction = -1;
		if(dim == 0) direction = 1;
	}
}

void led_on_pwm(int dim)
{
	PORTA = 0xff;
	
	for (int i=0; i<256; i++)
	{
		if (i>dim)
			PORTA = 0x00;	// duty cycle이 넘어가면 led off
		_delay_us(20);
	}
}