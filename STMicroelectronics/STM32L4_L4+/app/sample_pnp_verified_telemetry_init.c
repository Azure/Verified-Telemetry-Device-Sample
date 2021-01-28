/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>
#include "board_init.h"
#include "pnp_fallcurve_component.h"
#include "pnp_verified_telemetry.h"

#define NUMBER_OF_VT_ENABLED_TELEMETRIES                         2

#define FLASH_ADDRESS_ENABLED                                        0x080E0000

#define FLASH_ADDRESS_DISABLED                                        0x00

static VERIFIED_TELEMETRY_DB verified_telemetry_DB;

static PNP_FALLCURVE_COMPONENT sample_fallcurve_1;

static PNP_FALLCURVE_COMPONENT sample_fallcurve_2;

static PNP_FALLCURVE_COMPONENT *fallcurve_components[NUMBER_OF_VT_ENABLED_TELEMETRIES] = {NULL};

static CHAR *connected_sensors[NUMBER_OF_VT_ENABLED_TELEMETRIES] = {NULL};

void *sample_pnp_verified_telemetry_user_init()
{
    UINT status;
    if ((status = pnp_fallcurve_init(&sample_fallcurve_1, (UCHAR *)"vTaccelerometerXExternal", GPIOB,
                                            GPIO_PIN_8, &hadc1, ADC_CHANNEL_1, NULL, fallcurve_components,
                                            connected_sensors, (UCHAR *) "accelerometerXExternal", 
                                            NUMBER_OF_VT_ENABLED_TELEMETRIES)))
    {
        printf("Failed to initialize vTaccelerometerXExternal component: error code = 0x%08x\r\n", status);
    }

    else if ((status = pnp_fallcurve_init(&sample_fallcurve_2, (UCHAR *) "vTsoilMoistureExternal", GPIOB,
                                            GPIO_PIN_9, &hadc1, ADC_CHANNEL_2,NULL, fallcurve_components,
                                            connected_sensors, (UCHAR *) "soilMoistureExternal", 
                                            NUMBER_OF_VT_ENABLED_TELEMETRIES )))
    {
        printf("Failed to initialize vTsoilMoistureExternal component: error code = 0x%08x\r\n", status);
    }
    else if ((status = pnp_vt_init(&verified_telemetry_DB, fallcurve_components, NUMBER_OF_VT_ENABLED_TELEMETRIES, 
                                                true, FLASH_ADDRESS_DISABLED )))
    {
        printf("Failed to initialize Verified Telemetry: error code = 0x%08x\r\n", status);
    }
    return (void*)(&verified_telemetry_DB);
}