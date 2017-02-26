#ifndef __MAIN_H__
#define __MAIN_H__

typedef enum {
	NORMAL_START_MODE = 0,
	HELP_START_MODE = 1,
}E_start_mode;

typedef struct{
	bool verbose_mode;
	char pibot_ip[20];
	char input_keyboard[50];
}T_app_option;

typedef struct{
	bool is_initialized;
	T_app_option app_option;
	int uart_fd;
}T_pibot;

extern T_pibot g_pibot;

#endif
