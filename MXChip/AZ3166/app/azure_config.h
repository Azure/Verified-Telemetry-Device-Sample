/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

typedef enum
{
    None         = 0,
    WEP          = 1,
    WPA_PSK_TKIP = 2,
    WPA2_PSK_AES = 3
} WiFi_Mode;

// ----------------------------------------------------------------------------
// WiFi connection config
// ----------------------------------------------------------------------------
#define WIFI_SSID     "iot"
#define WIFI_PASSWORD "d3vic3s77"
#define WIFI_MODE     WPA2_PSK_AES

// ----------------------------------------------------------------------------
// Azure IoT Dynamic Provisioning Service
// Define this to use the DPS service, otherwise direct IoT Hub
// ----------------------------------------------------------------------------
//#define ENABLE_DPS

// ----------------------------------------------------------------------------
// Azure IoT Hub connection config
//    IOT_HUB_HOSTNAME: The Azure IoT Hub hostname
//    IOT_DEVICE_ID:    The Azure IoT Hub device id
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME "azuregsg.azure-devices.net"
#define IOT_DEVICE_ID    "mxchip_vt"

// ----------------------------------------------------------------------------
// Azure IoT DPS connection config
//    ID_SCOPE:        The DPS ID Scope
//    REGISTRATION_ID: The DPS device Registration Id
// ----------------------------------------------------------------------------
#define ID_SCOPE        ""
#define REGISTRATION_ID ""

// ----------------------------------------------------------------------------
// Azure IoT device SAS key
//    The SAS key generated by configuring an IoT Hub device or DPS individual
//    enrollment
// ----------------------------------------------------------------------------
#define DEVICE_SYMMETRIC_KEY "x0/QvUvyDiIB40z6urkQDtyL6PzdpmpGelTFwSb1AQE="

#endif // _AZURE_CONFIG_H
