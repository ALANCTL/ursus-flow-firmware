#ifndef __usb_device_H
#define __usb_device_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"
#include "usbd_def.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USB_Device init function */	
void MX_USB_DEVICE_Init(void);

#ifdef __cplusplus
}
#endif
#endif
