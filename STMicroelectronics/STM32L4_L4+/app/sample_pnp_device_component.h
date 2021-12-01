/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef SAMPLE_PNP_DEVICE_COMPONENT_H
#define SAMPLE_PNP_DEVICE_COMPONENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "nx_api.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_json_writer.h"
#include "nx_azure_iot_pnp_client.h"
#include "nx_verified_telemetry.h"

    typedef struct SAMPLE_PNP_DEVICE_COMPONENT_TAG
    {
        /* Name of this component */
        UCHAR* component_name_ptr;

        UINT component_name_length;

        /* Soil Moisture Sensor 1 data of this device component */
        double soilMoistureExternal1Raw;

        /* Humidity Sensor (DHT22) data of this device component */
        double soilMoistureExternal2Raw;

        /* Temperature Sensor 1 data of this device component */
        double temperatureExternal1Raw;

        /* Temperature Sensor 2 data of this device component */
        double temperatureExternal2Raw;

        /* Temperature Sensor data of this device component */
        double sensorTemperature;

        /* Pressure Sensor data of this device component */
        double sensorPressure;

        /* Humidity Sensor data of this device component */
        double sensorHumidity;

        /* Accelerometer Sensor data of this device component */
        double sensorAcceleration;

        /* Magnetometer Sensor data of this device component */
        double sensorMagnetic;

        /* LED state of this device component */
        bool sensorLEDState;

        /* Pointer to Verified Telemetry DB */
        NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB;

    } SAMPLE_PNP_DEVICE_COMPONENT;

    UINT sample_pnp_device_init(SAMPLE_PNP_DEVICE_COMPONENT* handle,
        UCHAR* component_name_ptr,
        UINT component_name_length,
        double default_sensor_reading,
        NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB);

    UINT sample_pnp_device_process_command(SAMPLE_PNP_DEVICE_COMPONENT* handle,
        UCHAR* component_name_ptr,
        UINT component_name_length,
        UCHAR* pnp_command_name_ptr,
        UINT pnp_command_name_length,
        NX_AZURE_IOT_JSON_READER* json_reader_ptr,
        NX_AZURE_IOT_JSON_WRITER* json_response_ptr,
        UINT* status_code);

    UINT sample_pnp_device_telemetry_send(VT_UINT iterx,
        SAMPLE_PNP_DEVICE_COMPONENT* handle, NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr);

    UINT sample_pnp_device_led_state_property(
        SAMPLE_PNP_DEVICE_COMPONENT* handle, NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr);

#ifdef __cplusplus
}
#endif
#endif /* SAMPLE_PNP_DEVICE_COMPONENT_H */
