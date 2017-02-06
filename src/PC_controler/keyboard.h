#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__


#define KEY_Z_CODE 17
#define KEY_Q_CODE 30
#define KEY_S_CODE 31
#define KEY_D_CODE 32

#define KEY_I_CODE 23
#define KEY_J_CODE 36
#define KEY_K_CODE 37
#define KEY_L_CODE 38

typedef enum{
	E_KEY_ACTION_RELEASE  =0,
	E_KEY_ACTION_PUSH     =1,
	E_KEY_ACTION_MAINTAIN =2,
}E_key_action;


typedef struct{
	//Key to control pibot
	bool Z_press;
	bool Q_press;
	bool S_press;
	bool D_press;
	//Key to control camera
	bool I_press;
	bool J_press;
	bool K_press;
	bool L_press;
} key_status;

int keyboard_init(void);

#endif
