/*
 * button.c
 *
 * Created: 2024-04-25 오후 1:30:31
 *  Author: kccistc
 */ 
#include "button.h" 
void init_button(void);
void auto_mode_check();
unsigned char previous_button_status[BUTTON_NUMBER] =
{BUTTON_RELEASE};
	
int get_button(int button_pin, int button_num);

// 76543210
// DDRD &= 0x0f;
void init_button(void)
{
	BUTTON_DDR &= ~(1 << BUTTON1PIN);
}

int button1_state = 0;
extern void stop();

void auto_mode_check()
{
	if(get_button(BUTTON1PIN, BUTTON1))
	{
		button1_state = !button1_state;
	}
	if(button1_state)
	{
		PORTG |= 1<<3;	// led on
		stop();
	}
	else
	{
		PORTG &= ~(1<<3);	// led off
		
	}
}

// 버튼이 온전하게 눌렸다 떼어진 상태 이면 1을 리턴 한다. 
int get_button(int button_pin, int button_num)  // 예) BUTTON1 4
{         
	int current_state; 
	                                                   //         1
	current_state = BUTTON_PIN & (1 << button_pin);  //  버튼값을 읽는다. 
	if (current_state && previous_button_status[button_num]==BUTTON_RELEASE)  // 버튼이 처음 눌려진상태
	{
		_delay_ms(60);   // 노이즈가 지나가기를 기다린다 
		previous_button_status[button_num]=BUTTON_PRESS;  // 처음 눌려진 상태가 아니다
		return 0;        // 아직은 완전히 눌렸다 떼어진 상태가 아니다. 
	}     
	else if (previous_button_status[button_num]==BUTTON_PRESS && current_state==BUTTON_RELEASE)
	       // 버튼이 이전에 눌러진 상태였으며 현재는 떼어진 상태
	{
		previous_button_status[button_num]=BUTTON_RELEASE;   // 다음 버튼 상태를 체크하기위해 초기화
	   _delay_ms(60);   // 노이즈가 지나가기를 기다린다
	  return 1;    // 완전히 눌렸다 떼어진 상태임을 리턴
	}
	 
	 return 0;   // 아직 완전히 스위치를 눌렀다 뗀 상태가 아닌경우 나
	             // 스위치가 open 된 상태                   	
}