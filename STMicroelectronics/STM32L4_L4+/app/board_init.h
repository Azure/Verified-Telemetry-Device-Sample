/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _BOARD_INIT_H
#define _BOARD_INIT_H

#include "stm32l4xx_hal.h"

extern UART_HandleTypeDef UartHandle;
extern ADC_HandleTypeDef hadc1;

void board_init(void);
int hardware_rand(void);


#endif // _BOARD_INIT_H
