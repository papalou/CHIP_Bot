#ifndef __MAIN_H__
#define __MAIN_H__

typedef enum{
	E_MODE_BOT = 0,
	E_MODE_CONTROLER,
	E_MODE_HELP,
}E_app_start_mode;

typedef struct{
	bool verbose_mode;
	E_app_start_mode start_mode;
}T_app_option;

typedef struct{
	bool is_initialized;
	T_app_option app_option;
}T_app;

extern T_app g_app;

#endif
