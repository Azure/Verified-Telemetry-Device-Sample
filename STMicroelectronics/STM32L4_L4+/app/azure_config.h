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
#define WIFI_SSID     ""
#define WIFI_PASSWORD ""
#define WIFI_MODE     WPA2_PSK_AES

/* This sample uses Symmetric key (SAS) to connect to IoT Hub by default,
   simply defining USE_DEVICE_CERTIFICATE and setting your device certificate in sample_device_identity.c
   to connect to IoT Hub with x509 certificate. Set up X.509 security in your Azure IoT Hub,
   refer to https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-security-x509-get-started  */
/* #define USE_DEVICE_CERTIFICATE                      1  */

/*
TODO`s: Configure core settings of application for your IoTHub.
*/

// ----------------------------------------------------------------------------
// Azure IoT Dynamic Provisioning Service
// Define this to use the DPS service, otherwise direct IoT Hub
// ----------------------------------------------------------------------------
#define ENABLE_DPS

#ifndef ENABLE_DPS

/* Required when DPS is not used.  */
/* These values can be picked from device connection string which is of format : HostName=<host1>;DeviceId=<device1>;SharedAccessKey=<key1>
   IOT_HUB_HOSTNAME can be set to <host1>,
   IOT_DEVICE_ID can be set to <device1>,
   DEVICE_SYMMETRIC_KEY can be set to <key1>.  */
#ifndef IOT_HUB_HOSTNAME
#define IOT_HUB_HOSTNAME                                   ""
#endif /* IOT_HOST_NAME */

#ifndef IOT_DEVICE_ID
#define IOT_DEVICE_ID                                   ""
#endif /* IOT_DEVICE_ID */

#else /* !ENABLE_DPS */

/* Required when DPS is used.  */
#ifndef ENDPOINT
#define ENDPOINT                                    ""
#endif /* ENDPOINT */

#ifndef ID_SCOPE
#define ID_SCOPE                                    ""
#endif /* ID_SCOPE */

#ifndef REGISTRATION_ID
#define REGISTRATION_ID                             ""
#endif /* REGISTRATION_ID */

#endif /* ENABLE_DPS */

/* Optional SYMMETRIC KEY.  */
#ifndef DEVICE_SYMMETRIC_KEY
#define DEVICE_SYMMETRIC_KEY                        ""
#endif /* DEVICE_SYMMETRIC_KEY */

/* Optional module ID.  */
#ifndef MODULE_ID
#define MODULE_ID ""
#endif /* MODULE_ID */

#if (USE_DEVICE_CERTIFICATE == 1)

/* Using X509 certificate authenticate to connect to IoT Hub,
   set the device certificate as your device.  */

/* Device Key type. */
#ifndef DEVICE_KEY_TYPE
#define DEVICE_KEY_TYPE NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER
#endif /* DEVICE_KEY_TYPE */

#endif /* USE_DEVICE_CERTIFICATE */

/*
END TODO section
*/

/* Define the Azure RTOS IOT thread stack and priority.  */
#ifndef NX_AZURE_IOT_STACK_SIZE
#define NX_AZURE_IOT_STACK_SIZE (2048)
#endif /* NX_AZURE_IOT_STACK_SIZE */

#ifndef NX_AZURE_IOT_THREAD_PRIORITY
#define NX_AZURE_IOT_THREAD_PRIORITY (4)
#endif /* NX_AZURE_IOT_THREAD_PRIORITY */

#ifndef SAMPLE_MAX_BUFFER
#define SAMPLE_MAX_BUFFER (256)
#endif /* SAMPLE_MAX_BUFFER */

/* Define the sample thread stack and priority.  */
#ifndef SAMPLE_STACK_SIZE
#define SAMPLE_STACK_SIZE (2048)
#endif /* SAMPLE_STACK_SIZE */

#ifndef SAMPLE_THREAD_PRIORITY
#define SAMPLE_THREAD_PRIORITY (16)
#endif /* SAMPLE_THREAD_PRIORITY */

/* Define sample properties count. */
#define MAX_PROPERTY_COUNT 2

#endif // _AZURE_CONFIG_H
