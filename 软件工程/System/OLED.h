#ifndef __OLED_H
#define __OLED_H

#include "u8g2.h"

#define IS_HUWEI			1

#if IS_HUWEI == 1
void u8g2_Init(u8g2_t *u8g2);
void HW_I2C_Init(void);
#else
void SW_I2C_Init(void);
void u8g2_Init(u8g2_t *u8g2);
#endif

#endif
