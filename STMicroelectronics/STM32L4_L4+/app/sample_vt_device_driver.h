/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _SAMPLE_VT_DEVICE_DRIVER_H
#define _SAMPLE_VT_DEVICE_DRIVER_H

#include "stm32l4xx_hal.h"
#include "vt_device_driver.h"
#include <stdint.h>

#define CURRENTSENSE_EXTERNAL_ADC_REF_VOLT   5.0f
#define CURRENTSENSE_EXTERNAL_ADC_RESOLUTION 12.0f
#define CURRENTSENSE_SHUNT_RESISTOR          1.0f
#define CURRENTSENSE_OPAMP_GAIN              50.0f

/* ADC Definitions */
extern uint16_t vt_adc_id_sensor_1;
extern uint16_t vt_adc_id_sensor_2;
extern uint16_t vt_adc_id_sensor_3;
extern uint16_t vt_adc_id_sensor_4;

extern ADC_HandleTypeDef vt_adc_controller_sensor_1;
extern ADC_HandleTypeDef vt_adc_controller_sensor_2;
extern SPI_HandleTypeDef vt_adc_controller_sensor_3;
extern SPI_HandleTypeDef vt_adc_controller_sensor_4;

extern uint32_t vt_adc_channel_sensor_1;
extern uint32_t vt_adc_channel_sensor_2;
extern uint32_t vt_adc_channel_sensor_3;
extern uint32_t vt_adc_channel_sensor_4;

/* GPIO Definitions */
extern uint16_t vt_gpio_id_sensor_1;
extern uint16_t vt_gpio_id_sensor_2;

extern GPIO_TypeDef* vt_gpio_port_sensor_1;
extern GPIO_TypeDef* vt_gpio_port_sensor_2;

extern uint16_t vt_gpio_pin_sensor_1;
extern uint16_t vt_gpio_pin_sensor_2;



/* Define prototypes. */

uint16_t vt_adc_single_read_init(uint16_t adc_id, void* adc_controller, void* adc_channel, uint16_t* adc_resolution, float* adc_ref_volt);
uint16_t vt_adc_single_read(uint16_t adc_id, void* adc_controller, void* adc_channel);
uint16_t vt_gpio_on(uint16_t gpio_id, void* gpio_port, void* gpio_pin);
uint16_t vt_gpio_off(uint16_t gpio_id, void* gpio_port, void* gpio_pin);
uint16_t vt_tick_init(uint16_t* max_value, uint16_t* resolution_usec);
unsigned long vt_tick_deinit();
unsigned long vt_tick();
void vt_interrupt_enable();
void vt_interrupt_disable();

#endif // _SAMPLE_VT_DEVICE_DRIVER_H
