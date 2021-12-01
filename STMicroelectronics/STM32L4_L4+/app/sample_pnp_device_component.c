/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "sample_pnp_device_component.h"
#include "board_init.h"
#include "stm32l475e_iot01.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_tsensor.h"
#include <math.h>

#define DOUBLE_DECIMAL_PLACE_DIGITS   (2)
#define SAMPLE_COMMAND_SUCCESS_STATUS (200)
#define SAMPLE_COMMAND_ERROR_STATUS   (500)

#define DS18B20_1_PORT GPIOA
#define DS18B20_1_PIN  GPIO_PIN_15
#define DS18B20_2_PORT GPIOB
#define DS18B20_2_PIN  GPIO_PIN_2

#define US_100_PORT GPIOB
#define US_100_PIN  GPIO_PIN_2

#define UART_BUFFER_LENGTH 100

/* Telemetry key */
static const CHAR telemetry_name_soilMoistureExternal1Raw[] = "soilMoistureExternal1";
static const CHAR telemetry_name_soilMoistureExternal2Raw[] = "soilMoistureExternal2";
static const CHAR telemetry_name_temperatureExternal1Raw[]  = "PMSExternal1";
static const CHAR telemetry_name_temperatureExternal2Raw[]  = "temperatureExternal2";
static const CHAR telemetry_name_sensorTemperature[]        = "temperature";
static const CHAR telemetry_name_sensorPressure[]           = "pressure";
static const CHAR telemetry_name_sensorHumidity[]           = "humidityPercentage";
static const CHAR telemetry_name_sensorAcceleration[]       = "acceleration";
static const CHAR telemetry_name_sensorMagnetic[]           = "magnetic";

/* Pnp command supported */
static const CHAR set_led_state[] = "setLedState";

/* Names of properties for desired/reporting */
static const CHAR reported_led_state[] = "ledState";

static UCHAR scratch_buffer[512];
uint8_t UART4_rxBuffer[UART_BUFFER_LENGTH];
//uint8_t UART4_txBuffer[UART_BUFFER_LENGTH];
//int pm_status;
//float val;



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



// static void set_pin_output(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
// {
//     GPIO_InitTypeDef GPIO_InitStruct = {0};

//     GPIO_InitStruct.Pin   = GPIO_Pin;
//     GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
//     GPIO_InitStruct.Pull  = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//     HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
// }

// static void set_pin_input(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
// {
//     GPIO_InitTypeDef GPIO_InitStruct = {0};

//     GPIO_InitStruct.Pin   = GPIO_Pin;
//     GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
//     GPIO_InitStruct.Pull  = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//     HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
// }

// static void delay_usec(uint32_t delay_usec)
// {
//     TIM_HandleTypeDef delay_usec_timer;
//     delay_usec_timer.Instance               = TIM7;
//     delay_usec_timer.Init.Prescaler         = (uint32_t)((SystemCoreClock / 1000000) - 1);
//     delay_usec_timer.Init.CounterMode       = TIM_COUNTERMODE_UP;
//     delay_usec_timer.Init.Period            = 65535;
//     delay_usec_timer.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
//     delay_usec_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//     if (HAL_TIM_Base_Init(&delay_usec_timer) != HAL_OK)
//     {
//         // add error handling
//     }
//     HAL_TIM_Base_Start(&delay_usec_timer);
//     while ((__HAL_TIM_GET_COUNTER(&delay_usec_timer)) < delay_usec)
//         ;
// }

// static uint8_t ds18b20_start(GPIO_TypeDef* ds18b20_port, uint16_t ds18b20_pin)
// {
//     uint8_t response = 0;
//     set_pin_output(ds18b20_port, ds18b20_pin);       // set the pin as output
//     HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, 0); // pull the pin low
//     delay_usec(480);                                 // delay according to datasheet

//     set_pin_input(ds18b20_port, ds18b20_pin); // set the pin as input
//     delay_usec(80);                           // delay according to datasheet

//     if (!(HAL_GPIO_ReadPin(ds18b20_port, ds18b20_pin)))
//     {
//         response = 1; // if the pin is low i.e the presence pulse is detected
//     }
//     else
//     {
//         response = 0;
//     }

//     delay_usec(400); // 480 us delay totally.
//     return response;
// }

// static void ds18b20_write(GPIO_TypeDef* ds18b20_port, uint16_t ds18b20_pin, uint8_t data)
// {
//     set_pin_output(ds18b20_port, ds18b20_pin); // set as output

//     for (int i = 0; i < 8; i++)
//     {
//         if ((data & (1 << i)) != 0) // if the bit is high
//         {
//             // write 1
//             set_pin_output(ds18b20_port, ds18b20_pin);       // set as output
//             HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, 0); // pull the pin LOW
//             delay_usec(1);                                   // wait for 1 us

//             set_pin_input(ds18b20_port, ds18b20_pin); // set as input
//             delay_usec(50);                           // wait for 60 us
//         }

//         else // if the bit is low
//         {
//             // write 0
//             set_pin_output(ds18b20_port, ds18b20_pin);
//             HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, 0); // pull the pin LOW
//             delay_usec(50);                                  // wait for 60 us

//             set_pin_input(ds18b20_port, ds18b20_pin);
//         }
//     }
// }

// static uint8_t ds18b20_read(GPIO_TypeDef* ds18b20_port, uint16_t ds18b20_pin)
// {
//     uint8_t value = 0;
//     set_pin_input(ds18b20_port, ds18b20_pin);

//     for (int i = 0; i < 8; i++)
//     {
//         set_pin_output(ds18b20_port, ds18b20_pin); // set as output

//         HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, 0); // pull the data pin LOW
//         delay_usec(2);                                   // wait for 2 us

//         set_pin_input(ds18b20_port, ds18b20_pin);        // set as input
//         if (HAL_GPIO_ReadPin(ds18b20_port, ds18b20_pin)) // if the pin is HIGH
//         {
//             value |= 1 << i; // read = 1
//         }
//         delay_usec(60); // wait for 60 us
//     }
//     return value;
// }

// static float ds18b20_temperature_read(GPIO_TypeDef* ds18b20_port, uint16_t ds18b20_pin)
// {
//     uint8_t byte1 = 0;
//     uint8_t byte2 = 0;
//     float integer;
//     float decimal;
//     if (ds18b20_start(ds18b20_port, ds18b20_pin))
//     {
//         HAL_Delay(1);
//         ds18b20_write(ds18b20_port, ds18b20_pin, 0xCC); // skip ROM
//         ds18b20_write(ds18b20_port, ds18b20_pin, 0x44); // convert t
//         HAL_Delay(800);

//         if (ds18b20_start(ds18b20_port, ds18b20_pin))
//         {
//             HAL_Delay(1);
//             ds18b20_write(ds18b20_port, ds18b20_pin, 0xCC); // skip ROM
//             ds18b20_write(ds18b20_port, ds18b20_pin, 0xBE); // Read Scratch-pad

//             byte1 = ds18b20_read(ds18b20_port, ds18b20_pin);
//             byte2 = ds18b20_read(ds18b20_port, ds18b20_pin);

//             integer = (int8_t)((byte1 >> 4) | (byte2 << 4));
//             decimal = (float)(byte1 & 0x0F) * 0.0625f;

//             return (integer + decimal);
//         }
//     }
//     return (0);
// }
VT_VOID co2_start_measurement(){

    uint8_t UART4_txBuffer[]={0x61,0x06,0x00,0x36,0x00,0x00,0x60,0x64};
            for (int i =0;i <UART_BUFFER_LENGTH;i++){
        UART4_rxBuffer[i]=0;
    }
   
    HAL_UART_Transmit(&UartHandle4, (uint8_t*)UART4_txBuffer, sizeof(UART4_txBuffer), 1000);
    HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, UART_BUFFER_LENGTH, 3000);



    printf("sent start\n");
            for (int j=0;j<UART_BUFFER_LENGTH;j++){

            printf("%x-", UART4_rxBuffer[j]);
        }
    #if VT_LOG_LEVEL > 2
    VTLogDebugNoTag("sent start\n");
    VTLogDebugNoTag("received packet:\n");
    
        for (int j=0;j<UART_BUFFER_LENGTH;j++){
            VTLogDebugNoTag("%x-", UART4_rxBuffer[j]);
        }
       VTLogDebugNoTag("\n");
       #endif

}

VT_VOID co2_stop_measurement(){

    uint8_t UART4_txBuffer[]={0x61,0x06,0x00,0x37,0x00,0x01,0xF0,0x64};

            for (int i =0;i <UART_BUFFER_LENGTH;i++){
        UART4_rxBuffer[i]=0;
    }
    HAL_UART_Transmit(&UartHandle4, UART4_txBuffer, sizeof(UART4_txBuffer), 1000);
        HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, UART_BUFFER_LENGTH, 3000);
       
           printf("sent stop\n");
            for (int j=0;j<UART_BUFFER_LENGTH;j++){

            printf("%x-", UART4_rxBuffer[j]);
        }

    #if VT_LOG_LEVEL > 2
    VTLogDebugNoTag("sent stop\n");
    VTLogDebugNoTag("received packet:\n");
    
        for (int j=0;j<UART_BUFFER_LENGTH;j++){
            VTLogDebugNoTag("%x-", UART4_rxBuffer[j]);
        }
       VTLogDebugNoTag("\n");
       #endif


}

VT_VOID co2_read_measurement(){

    uint8_t UART4_txBuffer[]={0x61,0x03,0x00,0x27,0x00,0x01,0x3d,0xa1};

            for (int i =0;i <UART_BUFFER_LENGTH;i++){
        UART4_rxBuffer[i]=0;
    }
    HAL_UART_Transmit(&UartHandle4, UART4_txBuffer, sizeof(UART4_txBuffer), 1000);
        HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, UART_BUFFER_LENGTH, 3000);
       
           printf("sent read ready\n");
            for (int j=0;j<UART_BUFFER_LENGTH;j++){

            printf("%x-", UART4_rxBuffer[j]);
        }

uint8_t UART4_txBuffer2[]={0x61,0x03,0x00,0x28,0x00,0x06,0x4c,0x60};

            for (int i =0;i <UART_BUFFER_LENGTH;i++){
        UART4_rxBuffer[i]=0;
    }
    HAL_UART_Transmit(&UartHandle4, UART4_txBuffer2, sizeof(UART4_txBuffer2), 1000);
        HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, UART_BUFFER_LENGTH, 4000);
       
           printf("sent read\n");
            for (int j=0;j<UART_BUFFER_LENGTH;j++){

            printf("%x-", UART4_rxBuffer[j]);
        }

    #if VT_LOG_LEVEL > 2
    VTLogDebugNoTag("sent stop\n");
    VTLogDebugNoTag("received packet:\n");
    
        for (int j=0;j<UART_BUFFER_LENGTH;j++){
            VTLogDebugNoTag("%x-", UART4_rxBuffer[j]);
        }
       VTLogDebugNoTag("\n");
       #endif


}

VT_VOID hpma_start_measurement(){

    uint8_t UART4_txBuffer[]={0x68,0x01,0x01,0x96};
            for (int i =0;i <UART_BUFFER_LENGTH;i++){
        UART4_rxBuffer[i]=0;
    }
   
    HAL_UART_Transmit(&UartHandle4, (uint8_t*)UART4_txBuffer, sizeof(UART4_txBuffer), 1000);
    HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, UART_BUFFER_LENGTH, 3000);



    printf("sent start\n");
            for (int j=0;j<UART_BUFFER_LENGTH;j++){

            printf("%x-", UART4_rxBuffer[j]);
        }
    #if VT_LOG_LEVEL > 2
    VTLogDebugNoTag("sent start\n");
    VTLogDebugNoTag("received packet:\n");
    
        for (int j=0;j<UART_BUFFER_LENGTH;j++){
            VTLogDebugNoTag("%x-", UART4_rxBuffer[j]);
        }
       VTLogDebugNoTag("\n");
       #endif

}

VT_VOID hpma_stop_measurement(){

    uint8_t UART4_txBuffer[]={0x68,0x01,0x02,0x95};

            for (int i =0;i <UART_BUFFER_LENGTH;i++){
        UART4_rxBuffer[i]=0;
    }
    HAL_UART_Transmit(&UartHandle4, UART4_txBuffer, sizeof(UART4_txBuffer), 1000);
        HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, UART_BUFFER_LENGTH, 3000);
       
           printf("sent stop\n");
            for (int j=0;j<UART_BUFFER_LENGTH;j++){

            printf("%x-", UART4_rxBuffer[j]);
        }

    #if VT_LOG_LEVEL > 2
    VTLogDebugNoTag("sent stop\n");
    VTLogDebugNoTag("received packet:\n");
    
        for (int j=0;j<UART_BUFFER_LENGTH;j++){
            VTLogDebugNoTag("%x-", UART4_rxBuffer[j]);
        }
       VTLogDebugNoTag("\n");
       #endif


}


VT_VOID sps30_start_measurement(){

    uint8_t UART4_txBuffer[]={0x7E,0x00,0x00,0x02,0x01,0x03,0xF9,0x7E};
   
    HAL_UART_Transmit(&UartHandle4, (uint8_t*)UART4_txBuffer, sizeof(UART4_txBuffer), 1000);
    HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, UART_BUFFER_LENGTH, 3000);
        // printf("sent start:\n");
    
        // for (int j=0;j<UART_BUFFER_LENGTH;j++){
        //     printf("%x-", UART4_rxBuffer[j]);
        // }

    #if VT_LOG_LEVEL > 2
    VTLogDebugNoTag("sent start\n");
    VTLogDebugNoTag("received packet:\n");
    
        for (int j=0;j<UART_BUFFER_LENGTH;j++){
            VTLogDebugNoTag("%x-", UART4_rxBuffer[j]);
        }
       VTLogDebugNoTag("\n");
       #endif

}

VT_VOID sps30_stop_measurement(){

    uint8_t UART4_txBuffer[]={0x7E,0x00,0x01,0x00,0xFE,0x7E};

    
    HAL_UART_Transmit(&UartHandle4, UART4_txBuffer, sizeof(UART4_txBuffer), 1000);
        HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, UART_BUFFER_LENGTH, 3000);
        //       printf("sent sotp:\n");
    
        // for (int j=0;j<UART_BUFFER_LENGTH;j++){
        //     printf("%x-", UART4_rxBuffer[j]);
        // }
    #if VT_LOG_LEVEL > 2
    VTLogDebugNoTag("sent stop\n");
    VTLogDebugNoTag("received packet:\n");
    
        for (int j=0;j<UART_BUFFER_LENGTH;j++){
            VTLogDebugNoTag("%x-", UART4_rxBuffer[j]);
        }
       VTLogDebugNoTag("\n");
       #endif


}

VT_VOID sps30_read_measurement(){
        union {
        char c[4];
        float f;
    } u;

    uint8_t UART4_txBuffer[]={0x7E,0x00,0x03,0x00,0xFC,0x7E};
        VT_INT decimal;
    VT_FLOAT frac_float;
    VT_INT frac;

    
    HAL_UART_Transmit(&UartHandle4, UART4_txBuffer, sizeof(UART4_txBuffer), 1000);
        HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, UART_BUFFER_LENGTH, 3000);
              printf("sent read:\n");
    
        for (int j=0;j<UART_BUFFER_LENGTH;j++){
            printf("%x-", UART4_rxBuffer[j]);
        }

    for (VT_UINT iter=0;iter<UART_BUFFER_LENGTH-31;iter++){
            if (UART4_rxBuffer[iter]==0x7e){
                if (UART4_rxBuffer[iter+1]==0x00){
                        u.c[3] = UART4_rxBuffer[iter+9];
                        u.c[2] = UART4_rxBuffer[iter+10];
                        u.c[1] = UART4_rxBuffer[iter+11];
                        u.c[0] = UART4_rxBuffer[iter+12];
                        break;
                }}}

            decimal    = u.f;
        frac_float = u.f - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
       printf("\npm2.5 val %d.%04d : \n", decimal, frac);

    
    #if VT_LOG_LEVEL > 2
    VTLogDebugNoTag("sent stop\n");
    VTLogDebugNoTag("received packet:\n");
    
        for (int j=0;j<UART_BUFFER_LENGTH;j++){
            VTLogDebugNoTag("%x-", UART4_rxBuffer[j]);
        }
       VTLogDebugNoTag("\n");
       #endif


}



VT_UINT getpmdata()
{
    uint16_t _checksum;
    uint16_t _calculatedChecksum;
    int flag=0;
    VT_UINT val1=0;

    for (int i =0;i <UART_BUFFER_LENGTH;i++){
        UART4_rxBuffer[i]=0;
    }

   // HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, 1, 5000);
        //printf(" UART4_rxBuffer: %x\n", *UART4_rxBuffer);
    //if((*UART4_rxBuffer == 0x42) || (*UART4_rxBuffer == 0x52))
    //{
        HAL_UART4_Receive (&UartHandle4, UART4_rxBuffer, UART_BUFFER_LENGTH, 2000);
        //HAL_UART_Transmit(&UartHandle4, UART4_rxBuffer, sizeof(UART4_rxBuffer), 1000);
        for (int j=0;j<UART_BUFFER_LENGTH;j++){
            printf("%x-", UART4_rxBuffer[j]);
        }
       // printf("\n");
        for (VT_UINT iter=0;iter<UART_BUFFER_LENGTH-31;iter++){
            if (UART4_rxBuffer[iter]==0x42){
                if (UART4_rxBuffer[iter+1]==0x4d){
                    for (VT_UINT iter2=0;iter2<30;iter2++){
                        
                        _calculatedChecksum += UART4_rxBuffer[iter+iter2];

                    }
                    _checksum = UART4_rxBuffer[iter+30] << 8;
                    _checksum |= UART4_rxBuffer[iter+31];

                    #if VT_LOG_LEVEL > 2
                    VTLogDebug("_checksum: %x\n", _checksum);
                    VTLogDebug("_calculatedChecksum: %x\n", _calculatedChecksum);
                    #endif

                    if (_checksum==_calculatedChecksum){
                         val1=  ((UART4_rxBuffer[iter+12]) << 8 | (UART4_rxBuffer[iter+13]));
                        printf("\nPMS Sensor Val: %d\n", val1);
                        flag=1;
                        break;
                    }
                    else{
                        _calculatedChecksum=0;
                    }

                }

            }



        }
    if (flag==0){printf("Error in getting PM2.5 value");}

    return val1;
}

// static void US_100_Ultrasonic_sensor(GPIO_TypeDef* US_100_port, uint16_t US_100_pin)
// {

//     set_pin_output(US_100_port, US_100_pin);       // set the pin as output
//     HAL_GPIO_WritePin(US_100_port, US_100_pin, 1); // pull the pin low
//     delay_usec(20);
//     HAL_GPIO_WritePin(US_100_port, US_100_pin, 0); // pull the pin low
//     delay_usec(20);

// }



UINT adc_read(ADC_HandleTypeDef* ADC_Controller, UINT ADC_Channel)
{
    UINT value                     = 0;
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel      = ADC_Channel;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset       = 0;

    HAL_ADC_ConfigChannel(ADC_Controller, &sConfig);

    HAL_ADC_Start(ADC_Controller);
    if (HAL_ADC_PollForConversion(ADC_Controller, 10) == HAL_OK)
    {
        value = HAL_ADC_GetValue(ADC_Controller);
    }
    HAL_ADC_Stop(ADC_Controller);
    HAL_Delay(200);

    return value;
}

/* Implementation of Set LED state command of device component  */
static UINT sample_pnp_device_set_led_state_command(SAMPLE_PNP_DEVICE_COMPONENT* handle,
    NX_AZURE_IOT_JSON_READER* json_reader_ptr,
    NX_AZURE_IOT_JSON_WRITER* out_json_writer_ptr)
{
    UINT state;
    UINT status;
    if ((status = nx_azure_iot_json_reader_next_token(json_reader_ptr)) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_bool_get(json_reader_ptr, &state))
        {
            return (NX_NOT_SUCCESSFUL);
        }
    }
    else
    {
        if (status != NX_AZURE_IOT_EMPTY_JSON)
        {
            return (NX_NOT_SUCCESSFUL);
        }
    }
    set_led_state_action((bool)state);
    handle->sensorLEDState = state;
    return (NX_AZURE_IOT_SUCCESS);
}

UINT sample_pnp_device_init(SAMPLE_PNP_DEVICE_COMPONENT* handle,
    UCHAR* component_name_ptr,
    UINT component_name_length,
    double default_sensor_reading,
    NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB)
{
    if (handle == NX_NULL)
    {
        return (NX_NOT_SUCCESSFUL);
    }

    handle->component_name_ptr       = component_name_ptr;
    handle->component_name_length    = component_name_length;
    handle->soilMoistureExternal1Raw = default_sensor_reading;
    handle->soilMoistureExternal2Raw = default_sensor_reading;
    handle->sensorTemperature        = default_sensor_reading;
    handle->sensorPressure           = default_sensor_reading;
    handle->sensorHumidity           = default_sensor_reading;
    handle->sensorAcceleration       = default_sensor_reading;
    handle->sensorMagnetic           = default_sensor_reading;
    handle->sensorLEDState           = false;
    handle->verified_telemetry_DB    = verified_telemetry_DB;

    return (NX_AZURE_IOT_SUCCESS);
}

UINT get_sensor_data(VT_UINT iterx,SAMPLE_PNP_DEVICE_COMPONENT* handle)
{
    if (handle == NX_NULL)
    {
        return (NX_NOT_SUCCESSFUL);
    }

    //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
    //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    //HAL_Delay(10);

    UINT soilMoisture1ADCData = adc_read(&hadc1, ADC_CHANNEL_1);
    UINT soilMoisture2ADCData = adc_read(&hadc1, ADC_CHANNEL_2);
    
    
    printf("******* CS PART *******\n");

    //for (int i=0;i<5;i++){

    //sps30_start_measurement();
    //hpma_start_measurement();
    //co2_start_measurement();

    //for (int i=0;i<5;i++){


    nx_vt_signature_read(handle->verified_telemetry_DB,
        (UCHAR*)telemetry_name_temperatureExternal1Raw,
        sizeof(telemetry_name_temperatureExternal1Raw) - 1);
    

    // int i=0;
    // while(i<1000000){
    //     i++;
    // }
    
    //float temperatureExternal1 = ds18b20_temperature_read(DS18B20_1_PORT, DS18B20_1_PIN);
    float pms_extrernal1= (float) getpmdata();
    //printf("%.2f",pms_extrernal1);

    // int i=0;
    // while(i<100000){
    //     i++;
    // }
    //co2_read_measurement();

    //sps30_read_measurement();

    //float temperatureExternal1 = ds18b20_temperature_read(DS18B20_1_PORT, DS18B20_1_PIN);
    //float pms_extrernal1= (float) getpmdata();
    //printf("%.2f",pms_extrernal1);




    nx_vt_signature_process(handle->verified_telemetry_DB,
        (UCHAR*)telemetry_name_temperatureExternal1Raw,
        sizeof(telemetry_name_temperatureExternal1Raw) - 1);
      //  }


    printf("\n******* CS PART END*******\n");
    



    float temperature = BSP_TSENSOR_ReadTemp();
    float humidity    = BSP_HSENSOR_ReadHumidity();
    float pressure    = BSP_PSENSOR_ReadPressure();
    int16_t magnetoXYZ[3];
    BSP_MAGNETO_GetXYZ(magnetoXYZ);
    int16_t accXYZ[3];
    BSP_ACCELERO_AccGetXYZ(accXYZ);

    handle->soilMoistureExternal1Raw = soilMoisture1ADCData;
    handle->soilMoistureExternal2Raw = soilMoisture2ADCData;

    handle->temperatureExternal1Raw  = pms_extrernal1;
    handle->temperatureExternal2Raw  = 30;// temperatureExternal2;


    handle->sensorTemperature  = temperature;
    handle->sensorPressure     = pressure;
    handle->sensorHumidity     = humidity;
    handle->sensorAcceleration = accXYZ[0];
    handle->sensorMagnetic     = magnetoXYZ[0];

    return (NX_AZURE_IOT_SUCCESS);
}

UINT sample_pnp_device_process_command(SAMPLE_PNP_DEVICE_COMPONENT* handle,
    UCHAR* component_name_ptr,
    UINT component_name_length,
    UCHAR* pnp_command_name_ptr,
    UINT pnp_command_name_length,
    NX_AZURE_IOT_JSON_READER* json_reader_ptr,
    NX_AZURE_IOT_JSON_WRITER* json_response_ptr,
    UINT* status_code)
{
    UINT dm_status;

    if (handle == NX_NULL)
    {
        return (NX_NOT_SUCCESSFUL);
    }

    if (handle->component_name_length != component_name_length ||
        strncmp((CHAR*)handle->component_name_ptr, (CHAR*)component_name_ptr, component_name_length) != 0)
    {
        return (NX_NOT_SUCCESSFUL);
    }

    if (pnp_command_name_length != (sizeof(set_led_state) - 1) ||
        strncmp((CHAR*)pnp_command_name_ptr, (CHAR*)set_led_state, pnp_command_name_length) != 0)
    {
        printf(
            "PnP command=%.*s is not supported on device component\r\n", pnp_command_name_length, pnp_command_name_ptr);
        dm_status = 404;
    }
    else
    {
        dm_status = (sample_pnp_device_set_led_state_command(handle, json_reader_ptr, json_response_ptr) !=
                        NX_AZURE_IOT_SUCCESS)
                        ? SAMPLE_COMMAND_ERROR_STATUS
                        : SAMPLE_COMMAND_SUCCESS_STATUS;
    }

    *status_code = dm_status;

    return (NX_AZURE_IOT_SUCCESS);
}

UINT sample_pnp_device_telemetry_send(VT_UINT iterx,SAMPLE_PNP_DEVICE_COMPONENT* handle, NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr)
{
    UINT status;
    NX_AZURE_IOT_JSON_WRITER json_writer;
    UINT buffer_length;

    if (handle == NX_NULL)
    {
        return (NX_NOT_SUCCESSFUL);
    }

    /* Get sensor data. */
    if ((status = get_sensor_data(iterx,handle)))
    {
        printf("Fetching Sensor data failed!: error code = 0x%08x\r\n", status);
        return (status);
    }

    /* Build telemetry JSON payload */
    if (nx_azure_iot_json_writer_with_buffer_init(&json_writer, scratch_buffer, sizeof(scratch_buffer)))
    {
        printf("Telemetry message failed to build message\r\n");
        return (NX_NOT_SUCCESSFUL);
    }
    if (nx_azure_iot_json_writer_append_begin_object(&json_writer) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
            (UCHAR*)telemetry_name_soilMoistureExternal1Raw,
            sizeof(telemetry_name_soilMoistureExternal1Raw) - 1,
            handle->soilMoistureExternal1Raw,
            DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
            (UCHAR*)telemetry_name_soilMoistureExternal2Raw,
            sizeof(telemetry_name_soilMoistureExternal2Raw) - 1,
            handle->soilMoistureExternal2Raw,
            DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
            (UCHAR*)telemetry_name_temperatureExternal1Raw,
            sizeof(telemetry_name_temperatureExternal1Raw) - 1,
            handle->temperatureExternal1Raw,
            DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
            (UCHAR*)telemetry_name_temperatureExternal2Raw,
            sizeof(telemetry_name_temperatureExternal2Raw) - 1,
            handle->temperatureExternal2Raw,
            DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
            (UCHAR*)telemetry_name_sensorTemperature,
            sizeof(telemetry_name_sensorTemperature) - 1,
            handle->sensorTemperature,
            DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
            (UCHAR*)telemetry_name_sensorPressure,
            sizeof(telemetry_name_sensorPressure) - 1,
            handle->sensorPressure,
            DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
            (UCHAR*)telemetry_name_sensorHumidity,
            sizeof(telemetry_name_sensorHumidity) - 1,
            handle->sensorHumidity,
            DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
            (UCHAR*)telemetry_name_sensorAcceleration,
            sizeof(telemetry_name_sensorAcceleration) - 1,
            handle->sensorAcceleration,
            DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
            (UCHAR*)telemetry_name_sensorMagnetic,
            sizeof(telemetry_name_sensorMagnetic) - 1,
            handle->sensorMagnetic,
            DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        printf("Telemetry message failed to build message\r\n");
        nx_azure_iot_json_writer_deinit(&json_writer);
        return (NX_NOT_SUCCESSFUL);
    }

    buffer_length = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
    /* Create and send the telemetry message packet. */
    if ((status = nx_vt_verified_telemetry_message_create_send(handle->verified_telemetry_DB,
             iotpnp_client_ptr,
             handle->component_name_ptr,
             handle->component_name_length,
             NX_WAIT_FOREVER,
             (UCHAR*)scratch_buffer,
             buffer_length)))
    {
        printf("Verified Telemetry message create and send failed!: error code = 0x%08x\r\n", status);
        nx_azure_iot_json_writer_deinit(&json_writer);
        return (status);
    }

    nx_azure_iot_json_writer_deinit(&json_writer);
    printf("Component %.*s Telemetry message send: %.*s.\r\n\n",
        handle->component_name_length,
        handle->component_name_ptr,
        buffer_length,
        scratch_buffer);

    return (status);
}

UINT sample_pnp_device_led_state_property(
    SAMPLE_PNP_DEVICE_COMPONENT* handle, NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr)
{
    UINT status;
    UINT response_status = 0;
    NX_AZURE_IOT_JSON_WRITER json_writer;

    if ((status = nx_azure_iot_pnp_client_reported_properties_create(iotpnp_client_ptr, &json_writer, NX_WAIT_FOREVER)))
    {
        printf("Failed create reported properties: error code = 0x%08x\r\n", status);
        return (status);
    }

    if ((status = nx_azure_iot_pnp_client_reported_property_component_begin(
             iotpnp_client_ptr, &json_writer, handle->component_name_ptr, handle->component_name_length)) ||
        (status = nx_azure_iot_json_writer_append_property_with_bool_value(
             &json_writer, (const UCHAR*)reported_led_state, sizeof(reported_led_state) - 1, handle->sensorLEDState)) ||
        (status = nx_azure_iot_pnp_client_reported_property_component_end(iotpnp_client_ptr, &json_writer)))
    {
        printf("Failed to build reported property!: error code = 0x%08x\r\n", status);
        nx_azure_iot_json_writer_deinit(&json_writer);
        return (status);
    }

    if ((status = nx_azure_iot_pnp_client_reported_properties_send(
             iotpnp_client_ptr, &json_writer, NX_NULL, &response_status, NX_NULL, (5 * NX_IP_PERIODIC_RATE))))
    {
        printf("Device twin reported properties failed!: error code = 0x%08x\r\n", status);
        nx_azure_iot_json_writer_deinit(&json_writer);
        return (status);
    }

    nx_azure_iot_json_writer_deinit(&json_writer);

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("device twin report properties failed with code : %d\r\n", response_status);
        return (NX_NOT_SUCCESSFUL);
    }

    return (status);
}
