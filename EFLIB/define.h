#ifndef _DEFINE_H
#define _DEFINE_H

typedef enum
{
	FALSE=0,
	TRUE=!FALSE
}boolean;


typedef enum
{
	SYS_INIT = 0,
	SYS_IDLE = 1,
	SYS_USB_CONNECTED = 2,
	SYS_RECORDING = 3,
	SYS_PREPARE = 4,
	SYS_FORMAT = 5
}SYS_Status;

typedef enum
{
	INPUT_NONE		=	0,
	INPUT_SETTIME	=	1,
	INPUT_START		=	2,
	INPUT_REQDATA	=	3,
	INPUT_CLEAR		=	4,
	INPUT_OFF			= 5
} INPUT_TYPE;

#endif

