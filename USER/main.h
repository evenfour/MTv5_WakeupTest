#ifndef _MAIN_H
#define _MAIN_H

#include "stm32f4xx.h"
#include "EF_tick.h"
#include "EF_spiflash.h"
#include "EF_button.h"
#include "define.h"
#include "magnet.h"

#include "string.h"
#include "stdio.h"
#include "led.h"
#include "ff.h"
#include "nrf2401.h"
//#include "analog.h"
#include "ads8330.h"
#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
#include "usb_bsp.h"

void EF_OpenMassStorage(void);
void EF_CloseMassStorage(void);
void saveDataToFlash(void);

extern USB_OTG_CORE_HANDLE   USB_OTG_dev;
u8 USB_CONNECTED = 0;

#endif



