/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "sample_pnp_device_component.h"
#include "stm32l475e_iot01.h"
#include "board_init.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_tsensor.h"

#define DOUBLE_DECIMAL_PLACE_DIGITS                                     (2)
#define SAMPLE_COMMAND_SUCCESS_STATUS                                   (200)
#define SAMPLE_COMMAND_ERROR_STATUS                                     (500)

/* Telemetry key */
static const CHAR telemetry_name_soilMoistureExternalRaw[] = "soilMoistureExternal";
static const CHAR telemetry_name_accelerometerXExternalRaw[] = "accelerometerXExternal";
static const CHAR telemetry_name_sensorTemperature[] = "temperature";
static const CHAR telemetry_name_sensorPressure[] = "pressure";
static const CHAR telemetry_name_sensorHumidity[] = "humidityPercentage";
static const CHAR telemetry_name_sensorAcceleration[] = "acceleration";
static const CHAR telemetry_name_sensorMagnetic[] = "magnetic";

/* Pnp command supported */
static const CHAR set_led_state[] = "setLedState";

/* Names of properties for desired/reporting */
static const CHAR reported_led_state[] = "ledState";

static UCHAR scratch_buffer[512];

static void set_led_state_action(bool level)
{
    if (level)
    {
        printf("LED is turned ON\r\n");
        BSP_LED_On(LED_GREEN);
    }
    else
    {
        printf("LED is turned OFF\r\n");
        BSP_LED_Off(LED_GREEN);
    }
}

LONG adc_read(ADC_HandleTypeDef *ADC_Controller, UINT ADC_Channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel      = ADC_Channel;
    sConfig.Rank         = 1;
    // sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    HAL_ADC_ConfigChannel(ADC_Controller, &sConfig);

    HAL_ADC_Start(ADC_Controller);
    if (HAL_ADC_PollForConversion(ADC_Controller, 10) == HAL_OK)
    {
        return (float)(HAL_ADC_GetValue(ADC_Controller));
    }

    return -1;
}

/* Implementation of Set LED state command of device component  */
static UINT sample_pnp_device_set_led_state_command(SAMPLE_PNP_DEVICE_COMPONENT *handle,NX_AZURE_IOT_JSON_READER *json_reader_ptr,
                                                    NX_AZURE_IOT_JSON_WRITER *out_json_writer_ptr)
{
    UINT state;
    UINT status;
    if ((status = nx_azure_iot_json_reader_next_token(json_reader_ptr)) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_bool_get(json_reader_ptr, &state))
        {
            return(NX_NOT_SUCCESSFUL);
        }
    }
    else
    {
        if (status != NX_AZURE_IOT_EMPTY_JSON)
        {
            return(NX_NOT_SUCCESSFUL);
        }
    }
    set_led_state_action((bool) state);
    handle -> sensorLEDState = state;
    return(NX_AZURE_IOT_SUCCESS);
}

UINT sample_pnp_device_init(SAMPLE_PNP_DEVICE_COMPONENT *handle,
                                UCHAR *component_name_ptr, UINT component_name_length,
                                double default_sensor_reading)
{
    if (handle == NX_NULL)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    handle -> component_name_ptr = component_name_ptr;
    handle -> component_name_length = component_name_length;
    handle -> soilMoistureExternalRaw = default_sensor_reading;
    handle -> accelerometerXExternalRaw = default_sensor_reading;
    handle -> sensorTemperature = default_sensor_reading;
    handle -> sensorPressure = default_sensor_reading;
    handle -> sensorHumidity = default_sensor_reading;
    handle -> sensorAcceleration = default_sensor_reading;
    handle -> sensorMagnetic = default_sensor_reading;
    handle -> sensorLEDState = false;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT get_sensor_data(SAMPLE_PNP_DEVICE_COMPONENT *handle)
{
    if (handle == NX_NULL)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    UINT soilMoistureADCData = adc_read(&hadc1, ADC_CHANNEL_6);
    UINT accelerometerADCData = adc_read(&hadc1, ADC_CHANNEL_5);

    float temperature = BSP_TSENSOR_ReadTemp();
    float humidity = BSP_HSENSOR_ReadHumidity();
    float pressure = BSP_PSENSOR_ReadPressure();
    int16_t magnetoXYZ[3];
    BSP_MAGNETO_GetXYZ(magnetoXYZ);
    int16_t accXYZ[3];
    BSP_ACCELERO_AccGetXYZ(accXYZ);

    handle -> soilMoistureExternalRaw = soilMoistureADCData;
    handle -> accelerometerXExternalRaw = accelerometerADCData;

    handle -> sensorTemperature = temperature;
    handle -> sensorPressure = pressure;
    handle -> sensorHumidity = humidity;
    handle -> sensorAcceleration = accXYZ[0];
    handle -> sensorMagnetic = magnetoXYZ[0];

    return(NX_AZURE_IOT_SUCCESS);
}

UINT sample_pnp_device_process_command(SAMPLE_PNP_DEVICE_COMPONENT *handle,
                                           UCHAR *component_name_ptr, UINT component_name_length,
                                           UCHAR *pnp_command_name_ptr, UINT pnp_command_name_length,
                                           NX_AZURE_IOT_JSON_READER *json_reader_ptr,
                                           NX_AZURE_IOT_JSON_WRITER *json_response_ptr, UINT *status_code)
{
    UINT dm_status;

    if (handle == NX_NULL)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (handle -> component_name_length != component_name_length ||
        strncmp((CHAR *)handle -> component_name_ptr, (CHAR *)component_name_ptr, component_name_length) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (pnp_command_name_length != (sizeof(set_led_state) - 1) ||
        strncmp((CHAR *)pnp_command_name_ptr, (CHAR *)set_led_state, pnp_command_name_length) != 0)
    {
        printf("PnP command=%.*s is not supported on device component\r\n", pnp_command_name_length, pnp_command_name_ptr);
        dm_status = 404;
    }
    else
    {
        dm_status = (sample_pnp_device_set_led_state_command(handle, json_reader_ptr, json_response_ptr) != NX_AZURE_IOT_SUCCESS) ? SAMPLE_COMMAND_ERROR_STATUS :
                                                                                                SAMPLE_COMMAND_SUCCESS_STATUS;
    }

    *status_code = dm_status;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT sample_pnp_device_telemetry_send(SAMPLE_PNP_DEVICE_COMPONENT *handle, NX_AZURE_IOT_PNP_CLIENT *iotpnp_client_ptr)
{
    UINT status;
    NX_PACKET *packet_ptr;
    NX_AZURE_IOT_JSON_WRITER json_writer;
    UINT buffer_length;

    if (handle == NX_NULL)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Get sensor data. */
    if ((status = get_sensor_data(handle)))
    {
        printf("Fetching Sensor data failed!: error code = 0x%08x\r\n", status);
        return(status);
    }

    /* Create a telemetry message packet. */
    if ((status = nx_azure_iot_pnp_client_telemetry_message_create(iotpnp_client_ptr, handle -> component_name_ptr,
                                                                   handle -> component_name_length,
                                                                   &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Telemetry message create failed!: error code = 0x%08x\r\n", status);
        return(status);
    }

    /* Build telemetry JSON payload */
    if (nx_azure_iot_json_writer_with_buffer_init(&json_writer, scratch_buffer, sizeof(scratch_buffer)))
    {
        printf("Telemetry message failed to build message\r\n");
        nx_azure_iot_pnp_client_telemetry_message_delete(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }
    if(nx_azure_iot_json_writer_append_begin_object(&json_writer) ||
       nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
                                                                  (UCHAR *)telemetry_name_soilMoistureExternalRaw,
                                                                  sizeof(telemetry_name_soilMoistureExternalRaw) - 1,
                                                                  handle -> soilMoistureExternalRaw,
                                                                  DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
                                                                  (UCHAR *)telemetry_name_accelerometerXExternalRaw,
                                                                  sizeof(telemetry_name_accelerometerXExternalRaw) - 1,
                                                                  handle -> accelerometerXExternalRaw,
                                                                  DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
                                                                  (UCHAR *)telemetry_name_sensorTemperature,
                                                                  sizeof(telemetry_name_sensorTemperature) - 1,
                                                                  handle -> sensorTemperature,
                                                                  DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
                                                                  (UCHAR *)telemetry_name_sensorPressure,
                                                                  sizeof(telemetry_name_sensorPressure) - 1,
                                                                  handle -> sensorPressure,
                                                                  DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
                                                                  (UCHAR *)telemetry_name_sensorHumidity,
                                                                  sizeof(telemetry_name_sensorHumidity) - 1,
                                                                  handle -> sensorHumidity,
                                                                  DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
                                                                  (UCHAR *)telemetry_name_sensorAcceleration,
                                                                  sizeof(telemetry_name_sensorAcceleration) - 1,
                                                                  handle -> sensorAcceleration,
                                                                  DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
                                                                  (UCHAR *)telemetry_name_sensorMagnetic,
                                                                  sizeof(telemetry_name_sensorMagnetic) - 1,
                                                                  handle -> sensorMagnetic,
                                                                  DOUBLE_DECIMAL_PLACE_DIGITS) ||                                                                                                                                                                                                                                                                                                  
       nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        printf("Telemetry message failed to build message\r\n");
        nx_azure_iot_json_writer_deinit(&json_writer);
        nx_azure_iot_pnp_client_telemetry_message_delete(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    buffer_length = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
    if ((status = nx_azure_iot_pnp_client_telemetry_send(iotpnp_client_ptr, packet_ptr,
                                                         (UCHAR *)scratch_buffer, buffer_length, NX_WAIT_FOREVER)))
    {
        printf("Telemetry message send failed!: error code = 0x%08x\r\n", status);
        nx_azure_iot_json_writer_deinit(&json_writer);
        nx_azure_iot_pnp_client_telemetry_message_delete(packet_ptr);
        return(status);
    }

    nx_azure_iot_json_writer_deinit(&json_writer);
    printf("Component %.*s Telemetry message send: %.*s.\r\n", handle -> component_name_length,
           handle -> component_name_ptr, buffer_length, scratch_buffer);

    return(status);
}

UINT sample_pnp_device_led_state_property(SAMPLE_PNP_DEVICE_COMPONENT *handle, NX_AZURE_IOT_PNP_CLIENT *iotpnp_client_ptr)
{
    UINT status;
    UINT response_status = 0;
    NX_AZURE_IOT_JSON_WRITER json_writer;

    if ((status = nx_azure_iot_pnp_client_reported_properties_create(iotpnp_client_ptr,
                                                                     &json_writer, NX_WAIT_FOREVER)))
    {
        printf("Failed create reported properties: error code = 0x%08x\r\n", status);
        return(status);
    }

    if ((status = nx_azure_iot_pnp_client_reported_property_component_begin(iotpnp_client_ptr,
                                                                            &json_writer,
                                                                            handle -> component_name_ptr,
                                                                            handle -> component_name_length)) ||
        (status = nx_azure_iot_json_writer_append_property_with_bool_value(&json_writer,
                                                                             (const UCHAR *)reported_led_state,
                                                                             sizeof(reported_led_state) - 1,
                                                                             handle -> sensorLEDState)) ||
        (status = nx_azure_iot_pnp_client_reported_property_component_end(iotpnp_client_ptr,
                                                                          &json_writer)))
    {
        printf("Failed to build reported property!: error code = 0x%08x\r\n", status);
        nx_azure_iot_json_writer_deinit(&json_writer);
        return(status);
    }

    if ((status = nx_azure_iot_pnp_client_reported_properties_send(iotpnp_client_ptr,
                                                                   &json_writer,
                                                                   NX_NULL, &response_status,
                                                                   NX_NULL,
                                                                   (5 * NX_IP_PERIODIC_RATE))))
    {
        printf("Device twin reported properties failed!: error code = 0x%08x\r\n", status);
        nx_azure_iot_json_writer_deinit(&json_writer);
        return(status);
    }

    nx_azure_iot_json_writer_deinit(&json_writer);

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("device twin report properties failed with code : %d\r\n", response_status);
        return(NX_NOT_SUCCESSFUL);
    }

    return(status);
}

