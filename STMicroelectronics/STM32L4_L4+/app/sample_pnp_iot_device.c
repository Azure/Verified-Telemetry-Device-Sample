/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "nx_api.h"
#include "nx_azure_iot_pnp_client.h"
#include "nx_azure_iot_provisioning_client.h"

#include "azure_iot_cert.h"
#include "azure_iot_ciphersuites.h"
//#include "sample_config.h"
#include "azure_config.h"

#include "board_init.h"

/* Device */
#include "sample_pnp_device_component.h"

/* Verified Telemetry */
#include "pnp_middleware_helper.h"
#include "pnp_verified_telemetry.h"
#include "sample_pnp_verified_telemetry_init.h"

#define ENDPOINT                     "global.azure-devices-provisioning.net"
#define MODULE_ID                    ""
#define NX_AZURE_IOT_STACK_SIZE      (2048)
#define NX_AZURE_IOT_THREAD_PRIORITY (4)
#define SAMPLE_MAX_BUFFER            (256)

#ifndef SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC
#define SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC (10 * 60)
#endif /* SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC */

#ifndef SAMPLE_INITIAL_EXPONENTIAL_BACKOFF_IN_SEC
#define SAMPLE_INITIAL_EXPONENTIAL_BACKOFF_IN_SEC (3)
#endif /* SAMPLE_INITIAL_EXPONENTIAL_BACKOFF_IN_SEC */

#ifndef SAMPLE_MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT
#define SAMPLE_MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT (60)
#endif /* SAMPLE_MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT */

#ifndef SAMPLE_WAIT_OPTION
#define SAMPLE_WAIT_OPTION (NX_NO_WAIT)
#endif /* SAMPLE_WAIT_OPTION */

/* Sample events */
#define SAMPLE_ALL_EVENTS                       ((ULONG)0xFFFFFFFF)
#define SAMPLE_CONNECT_EVENT                    ((ULONG)0x00000001)
#define SAMPLE_INITIALIZATION_EVENT             ((ULONG)0x00000002)
#define SAMPLE_COMMAND_MESSAGE_EVENT            ((ULONG)0x00000004)
#define SAMPLE_DEVICE_PROPERTIES_GET_EVENT      ((ULONG)0x00000008)
#define SAMPLE_DEVICE_DESIRED_PROPERTIES_EVENT  ((ULONG)0x00000010)
#define SAMPLE_TELEMETRY_SEND_EVENT             ((ULONG)0x00000020)
#define SAMPLE_DEVICE_REPORTED_PROPERTIES_EVENT ((ULONG)0x00000040)
#define SAMPLE_DISCONNECT_EVENT                 ((ULONG)0x00000080)
#define SAMPLE_RECONNECT_EVENT                  ((ULONG)0x00000100)
#define SAMPLE_CONNECTED_EVENT                  ((ULONG)0x00000200)

/* Sample states */
#define SAMPLE_STATE_NONE         (0)
#define SAMPLE_STATE_INIT         (1)
#define SAMPLE_STATE_CONNECTING   (2)
#define SAMPLE_STATE_CONNECT      (3)
#define SAMPLE_STATE_CONNECTED    (4)
#define SAMPLE_STATE_DISCONNECTED (5)

#define SAMPLE_DEFAULT_DEVICE_SENSOR_READING (22)
#define DOUBLE_DECIMAL_PLACE_DIGITS          (2)
#define SAMPLE_COMMAND_SUCCESS_STATUS        (200)
#define SAMPLE_COMMAND_ERROR_STATUS          (500)
#define SAMPLE_COMMAND_NOT_FOUND_STATUS      (404)

#define SAMPLE_PNP_MODEL_ID    "dtmi:azure:verifiedtelemetry:sample:GSG;1"
#define SAMPLE_PNP_DPS_PAYLOAD "{\"modelId\":\"" SAMPLE_PNP_MODEL_ID "\"}"

/* Define Sample context.  */
typedef struct SAMPLE_CONTEXT_STRUCT
{
    UINT state;
    UINT action_result;
    ULONG last_periodic_action_tick;

    TX_EVENT_FLAGS_GROUP sample_events;

    /* Generally, IoTHub Client and DPS Client do not run at the same time, user can use union as below to
       share the memory between IoTHub Client and DPS Client.

       NOTE: If user can not make sure sharing memory is safe, IoTHub Client and DPS Client must be defined seperately.
     */
    union SAMPLE_CLIENT_UNION {
        NX_AZURE_IOT_PNP_CLIENT iotpnp_client;
#ifdef ENABLE_DPS
        NX_AZURE_IOT_PROVISIONING_CLIENT prov_client;
#endif /* ENABLE_DPS */
    } client;

#define iotpnp_client client.iotpnp_client
#ifdef ENABLE_DPS
#define prov_client client.prov_client
#endif /* ENABLE_DPS */

} SAMPLE_CONTEXT;

VOID sample_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time));

#ifdef ENABLE_DPS
static UINT sample_dps_entry(NX_AZURE_IOT_PROVISIONING_CLIENT* prov_client_ptr,
    UCHAR** iothub_hostname,
    UINT* iothub_hostname_length,
    UCHAR** iothub_device_id,
    UINT* iothub_device_id_length);
#endif /* ENABLE_DPS */

/* Define Azure RTOS TLS info.  */
static NX_SECURE_X509_CERT root_ca_cert;
static UCHAR nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE];
static ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];

/* Using X509 certificate authenticate to connect to IoT Hub,
   set the device certificate as your device.  */
#if (USE_DEVICE_CERTIFICATE == 1)
extern const UCHAR sample_device_cert_ptr[];
extern const UINT sample_device_cert_len;
extern const UCHAR sample_device_private_key_ptr[];
extern const UINT sample_device_private_key_len;
NX_SECURE_X509_CERT device_certificate;
#endif /* USE_DEVICE_CERTIFICATE */

/* Define buffer for IoTHub info. */
#ifdef ENABLE_DPS
static UCHAR sample_iothub_hostname[SAMPLE_MAX_BUFFER];
static UCHAR sample_iothub_device_id[SAMPLE_MAX_BUFFER];
#endif /* ENABLE_DPS */

/* Define the prototypes for AZ IoT.  */
static NX_AZURE_IOT nx_azure_iot;

static SAMPLE_CONTEXT sample_context;
static volatile UINT sample_connection_status = NX_NOT_CONNECTED;
static UINT exponential_retry_count;

static SAMPLE_PNP_DEVICE_COMPONENT sample_device;
static const CHAR sample_device_component[] = "sampleDevice";
bool sample_led_state_reported;
static UINT sample_device_properties_sent = 0;


static const CHAR sample_vTDevice_component[]    = "vTDevice";
static const CHAR sample_fallcurve_1_component[] = "vTsoilMoistureExternal2";
static const CHAR sample_fallcurve_2_component[] = "vTsoilMoistureExternal1";
static void* verified_telemetry_DB               = NULL;

static UCHAR scratch_buffer[2096];

static UINT exponential_backoff_with_jitter()
{
    double jitter_percent = (SAMPLE_MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT / 100.0) * (rand() / ((double)RAND_MAX));
    UINT base_delay       = SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC;
    uint64_t delay;

    if (exponential_retry_count < (sizeof(UINT) * 8))
    {
        delay = (uint64_t)((1 << exponential_retry_count) * SAMPLE_INITIAL_EXPONENTIAL_BACKOFF_IN_SEC);
        if (delay <= (UINT)(-1))
        {
            base_delay = (UINT)delay;
        }
    }

    if (base_delay > SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC)
    {
        base_delay = SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC;
    }
    else
    {
        exponential_retry_count++;
    }

    return ((UINT)(base_delay * (1 + jitter_percent)) * NX_IP_PERIODIC_RATE);
}

static VOID exponential_backoff_reset()
{
    exponential_retry_count = 0;
}

static VOID connection_status_callback(NX_AZURE_IOT_PNP_CLIENT* hub_client_ptr, UINT status)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);

    sample_connection_status = status;

    if (status)
    {
        printf("Disconnected from IoTHub!: error code = 0x%08x\r\n", status);
        tx_event_flags_set(&(sample_context.sample_events), SAMPLE_DISCONNECT_EVENT, TX_OR);
    }
    else
    {
        printf("Connected to IoTHub.\r\n");
        tx_event_flags_set(&(sample_context.sample_events), SAMPLE_CONNECTED_EVENT, TX_OR);
        exponential_backoff_reset();
    }
}

static VOID message_receive_callback_properties(NX_AZURE_IOT_PNP_CLIENT* hub_client_ptr, VOID* context)
{
    SAMPLE_CONTEXT* sample_ctx = (SAMPLE_CONTEXT*)context;

    NX_PARAMETER_NOT_USED(hub_client_ptr);
    tx_event_flags_set(&(sample_ctx->sample_events), SAMPLE_DEVICE_PROPERTIES_GET_EVENT, TX_OR);
}

static VOID message_receive_callback_command(NX_AZURE_IOT_PNP_CLIENT* hub_client_ptr, VOID* context)
{
    SAMPLE_CONTEXT* sample_ctx = (SAMPLE_CONTEXT*)context;

    NX_PARAMETER_NOT_USED(hub_client_ptr);
    tx_event_flags_set(&(sample_ctx->sample_events), SAMPLE_COMMAND_MESSAGE_EVENT, TX_OR);
}

static VOID message_receive_callback_desired_property(NX_AZURE_IOT_PNP_CLIENT* hub_client_ptr, VOID* context)
{
    SAMPLE_CONTEXT* sample_ctx = (SAMPLE_CONTEXT*)context;

    NX_PARAMETER_NOT_USED(hub_client_ptr);
    tx_event_flags_set(&(sample_ctx->sample_events), SAMPLE_DEVICE_DESIRED_PROPERTIES_EVENT, TX_OR);
}

static VOID sample_connect_action(SAMPLE_CONTEXT* context)
{
    if (context->state != SAMPLE_STATE_CONNECT)
    {
        return;
    }

    context->action_result = nx_azure_iot_pnp_client_connect(&(context->iotpnp_client), NX_FALSE, NX_WAIT_FOREVER);

    if (context->action_result == NX_AZURE_IOT_CONNECTING)
    {
        context->state = SAMPLE_STATE_CONNECTING;
    }
    else if (context->action_result != NX_SUCCESS)
    {
        sample_connection_status = context->action_result;
        context->state           = SAMPLE_STATE_DISCONNECTED;
    }
    else
    {
        context->state = SAMPLE_STATE_CONNECTED;

        context->action_result = nx_azure_iot_pnp_client_properties_request(&(context->iotpnp_client), NX_WAIT_FOREVER);
    }
}

static VOID sample_disconnect_action(SAMPLE_CONTEXT* context)
{
    if (context->state != SAMPLE_STATE_CONNECTED && context->state != SAMPLE_STATE_CONNECTING)
    {
        return;
    }

    context->action_result = nx_azure_iot_pnp_client_disconnect(&(context->iotpnp_client));
    context->state         = SAMPLE_STATE_DISCONNECTED;
}

static VOID sample_connected_action(SAMPLE_CONTEXT* context)
{
    if (context->state != SAMPLE_STATE_CONNECTING)
    {
        return;
    }

    context->state = SAMPLE_STATE_CONNECTED;

    context->action_result = nx_azure_iot_pnp_client_properties_request(&(context->iotpnp_client), NX_WAIT_FOREVER);
}

static VOID sample_initialize_iothub(SAMPLE_CONTEXT* context)
{
    UINT status;
#ifdef ENABLE_DPS
    UCHAR* iothub_hostname       = NX_NULL;
    UCHAR* iothub_device_id      = NX_NULL;
    UINT iothub_hostname_length  = 0;
    UINT iothub_device_id_length = 0;
#else
    UCHAR* iothub_hostname       = (UCHAR*)IOT_HUB_HOSTNAME;
    UCHAR* iothub_device_id      = (UCHAR*)IOT_DEVICE_ID;
    UINT iothub_hostname_length  = sizeof(IOT_HUB_HOSTNAME) - 1;
    UINT iothub_device_id_length = sizeof(IOT_DEVICE_ID) - 1;
#endif /* ENABLE_DPS */
    NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr = &(context->iotpnp_client);

    if (context->state != SAMPLE_STATE_INIT)
    {
        return;
    }

#ifdef ENABLE_DPS

    /* Run DPS.  */
    if ((status = sample_dps_entry(&(context->prov_client),
             &iothub_hostname,
             &iothub_hostname_length,
             &iothub_device_id,
             &iothub_device_id_length)))
    {
        printf("Failed on sample_dps_entry!: error code = 0x%08x\r\n", status);
        context->action_result = status;
        return;
    }
#endif /* ENABLE_DPS */

    printf("IoTHub Host Name: %.*s; Device ID: %.*s.\r\n",
        iothub_hostname_length,
        iothub_hostname,
        iothub_device_id_length,
        iothub_device_id);

    /* Initialize IoTHub client.  */
    if ((status = nx_azure_iot_pnp_client_initialize(iotpnp_client_ptr,
             &nx_azure_iot,
             iothub_hostname,
             iothub_hostname_length,
             iothub_device_id,
             iothub_device_id_length,
             (const UCHAR*)MODULE_ID,
             sizeof(MODULE_ID) - 1,
             (const UCHAR*)SAMPLE_PNP_MODEL_ID,
             sizeof(SAMPLE_PNP_MODEL_ID) - 1,
             _nx_azure_iot_tls_supported_crypto,
             _nx_azure_iot_tls_supported_crypto_size,
             _nx_azure_iot_tls_ciphersuite_map,
             _nx_azure_iot_tls_ciphersuite_map_size,
             nx_azure_iot_tls_metadata_buffer,
             sizeof(nx_azure_iot_tls_metadata_buffer),
             &root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_pnp_client_initialize!: error code = 0x%08x\r\n", status);
        context->action_result = status;
        return;
    }

#if (USE_DEVICE_CERTIFICATE == 1)

    /* Initialize the device certificate.  */
    if ((status = nx_secure_x509_certificate_initialize(&device_certificate,
             (UCHAR*)sample_device_cert_ptr,
             (USHORT)sample_device_cert_len,
             NX_NULL,
             0,
             (UCHAR*)sample_device_private_key_ptr,
             (USHORT)sample_device_private_key_len,
             DEVICE_KEY_TYPE)))
    {
        printf("Failed on nx_secure_x509_certificate_initialize!: error code = 0x%08x\r\n", status);
    }

    /* Set device certificate.  */
    else if ((status = nx_azure_iot_pnp_client_device_cert_set(iotpnp_client_ptr, &device_certificate)))
    {
        printf("Failed on nx_azure_iot_pnp_client_device_cert_set!: error code = 0x%08x\r\n", status);
    }
#else

    /* Set symmetric key.  */
    if ((status = nx_azure_iot_pnp_client_symmetric_key_set(
             iotpnp_client_ptr, (UCHAR*)DEVICE_SYMMETRIC_KEY, sizeof(DEVICE_SYMMETRIC_KEY) - 1)))
    {
        printf("Failed on nx_azure_iot_pnp_client_symmetric_key_set! error: 0x%08x\r\n", status);
    }
#endif /* USE_DEVICE_CERTIFICATE */

    /* Set connection status callback.  */
    else if ((status = nx_azure_iot_pnp_client_connection_status_callback_set(
                  iotpnp_client_ptr, connection_status_callback)))
    {
        printf("Failed on connection_status_callback!\r\n");
    }
    else if ((status = nx_azure_iot_pnp_client_receive_callback_set(
                  iotpnp_client_ptr, NX_AZURE_IOT_PNP_PROPERTIES, message_receive_callback_properties, (VOID*)context)))
    {
        printf("device properties callback set!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_pnp_client_receive_callback_set(
                  iotpnp_client_ptr, NX_AZURE_IOT_PNP_COMMAND, message_receive_callback_command, (VOID*)context)))
    {
        printf("device command callback set!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_pnp_client_receive_callback_set(iotpnp_client_ptr,
                  NX_AZURE_IOT_PNP_DESIRED_PROPERTIES,
                  message_receive_callback_desired_property,
                  (VOID*)context)))
    {
        printf("device desired property callback set!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_pnp_client_component_add(
                  iotpnp_client_ptr, (const UCHAR*)sample_device_component, sizeof(sample_device_component) - 1)) ||
             (status = nx_azure_iot_pnp_client_component_add(
                  iotpnp_client_ptr, (const UCHAR*)sample_vTDevice_component, sizeof(sample_vTDevice_component) - 1)) ||
             (status = nx_azure_iot_pnp_client_component_add(iotpnp_client_ptr,
                  (const UCHAR*)sample_fallcurve_1_component,
                  sizeof(sample_fallcurve_1_component) - 1)) ||
             (status = nx_azure_iot_pnp_client_component_add(iotpnp_client_ptr,
                  (const UCHAR*)sample_fallcurve_2_component,
                  sizeof(sample_fallcurve_2_component) - 1))
        //                                                  ||
        // (status = nx_azure_iot_pnp_client_component_add(iotpnp_client_ptr,
        //                                                  (const UCHAR *)sample_fallcurve_PC_component,
        //                                                  sizeof(sample_fallcurve_PC_component) - 1))
        //                                                  ||
        // (status = nx_azure_iot_pnp_client_component_add(iotpnp_client_ptr,
        //                                                  (const UCHAR *)sample_fallcurve_Cert_component,
        //                                                  sizeof(sample_fallcurve_Cert_component) - 1))
    )
    {
        printf("Failed to add component to pnp client!: error code = 0x%08x\r\n", status);
    }

    if (status)
    {
        nx_azure_iot_pnp_client_deinitialize(iotpnp_client_ptr);
    }

    context->action_result = status;

    if (status == NX_AZURE_IOT_SUCCESS)
    {
        context->state = SAMPLE_STATE_CONNECT;
    }
}

static VOID sample_connection_error_recover(SAMPLE_CONTEXT* context)
{
    if (context->state != SAMPLE_STATE_DISCONNECTED)
    {
        return;
    }

    switch (sample_connection_status)
    {
        case NX_AZURE_IOT_SUCCESS: {
            printf("already connected\r\n");
        }
        break;

        /* Something bad has happened with client state, we need to re-initialize it.  */
        case NX_DNS_QUERY_FAILED:
        case NXD_MQTT_ERROR_BAD_USERNAME_PASSWORD:
        case NXD_MQTT_ERROR_NOT_AUTHORIZED: {
            printf("re-initializing iothub connection, after backoff\r\n");

            tx_thread_sleep(exponential_backoff_with_jitter());
            nx_azure_iot_pnp_client_deinitialize(&(context->iotpnp_client));
            context->state = SAMPLE_STATE_INIT;
        }
        break;

        default: {
            printf("reconnecting iothub, after backoff\r\n");

            tx_thread_sleep(exponential_backoff_with_jitter());
            context->state = SAMPLE_STATE_CONNECT;
        }
        break;
    }
}

static VOID sample_trigger_action(SAMPLE_CONTEXT* context)
{
    switch (context->state)
    {
        case SAMPLE_STATE_INIT: {
            tx_event_flags_set(&(context->sample_events), SAMPLE_INITIALIZATION_EVENT, TX_OR);
        }
        break;

        case SAMPLE_STATE_CONNECT: {
            tx_event_flags_set(&(context->sample_events), SAMPLE_CONNECT_EVENT, TX_OR);
        }
        break;

        case SAMPLE_STATE_CONNECTED: {
            if ((tx_time_get() - context->last_periodic_action_tick) >= (5 * NX_IP_PERIODIC_RATE))
            {
                context->last_periodic_action_tick = tx_time_get();
                tx_event_flags_set(&(context->sample_events), SAMPLE_TELEMETRY_SEND_EVENT, TX_OR);
                tx_event_flags_set(&(context->sample_events), SAMPLE_DEVICE_REPORTED_PROPERTIES_EVENT, TX_OR);
            }
        }
        break;

        case SAMPLE_STATE_DISCONNECTED: {
            tx_event_flags_set(&(context->sample_events), SAMPLE_RECONNECT_EVENT, TX_OR);
        }
        break;
    }
}

static VOID sample_command_action(SAMPLE_CONTEXT* sample_context_ptr)
{
    UINT status;
    USHORT context_length;
    VOID* context_ptr;
    UINT component_name_length;
    const UCHAR* component_name_ptr;
    UINT pnp_command_name_length;
    const UCHAR* pnp_command_name_ptr;
    NX_AZURE_IOT_JSON_WRITER json_writer;
    NX_AZURE_IOT_JSON_READER json_reader;
    UINT status_code;
    UINT response_length;

    if (sample_context_ptr->state != SAMPLE_STATE_CONNECTED)
    {
        return;
    }

    if ((status = nx_azure_iot_pnp_client_command_receive(&(sample_context_ptr->iotpnp_client),
             &component_name_ptr,
             &component_name_length,
             &pnp_command_name_ptr,
             &pnp_command_name_length,
             &context_ptr,
             &context_length,
             &json_reader,
             NX_WAIT_FOREVER)))
    {
        printf("Command receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    if (component_name_ptr != NX_NULL)
    {
        printf("Received component: %.*s ", component_name_length, component_name_ptr);
    }
    else
    {
        printf("Received component: root component ");
    }

    printf("command: %.*s", pnp_command_name_length, (CHAR*)pnp_command_name_ptr);
    printf("\r\n");

    if ((status = nx_azure_iot_json_writer_with_buffer_init(&json_writer, scratch_buffer, sizeof(scratch_buffer))))
    {
        printf("Failed to initialize json builder response \r\n");
        nx_azure_iot_json_reader_deinit(&json_reader);
        return;
    }

    if ((status = sample_pnp_device_process_command(&sample_device,
             (UCHAR*)component_name_ptr,
             component_name_length,
             (UCHAR*)pnp_command_name_ptr,
             pnp_command_name_length,
             &json_reader,
             &json_writer,
             &status_code)) == NX_AZURE_IOT_SUCCESS)
    {
        printf("Successfully executed command %.*s on device component 1\r\n",
            pnp_command_name_length,
            pnp_command_name_ptr);
        response_length = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
    }
    else if ((status = pnp_vt_process_command(verified_telemetry_DB,
                  &(sample_context_ptr->iotpnp_client),
                  (UCHAR*)component_name_ptr,
                  component_name_length,
                  (UCHAR*)pnp_command_name_ptr,
                  pnp_command_name_length,
                  &json_reader,
                  &json_writer,
                  &status_code)) == NX_AZURE_IOT_SUCCESS)
    {
        response_length = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
    }
    else
    {
        printf("Failed to find any handler for command %.*s\r\n", pnp_command_name_length, pnp_command_name_ptr);
        status_code     = SAMPLE_COMMAND_NOT_FOUND_STATUS;
        response_length = 0;
    }

    nx_azure_iot_json_reader_deinit(&json_reader);

    if ((status = nx_azure_iot_pnp_client_command_message_response(&(sample_context_ptr->iotpnp_client),
             status_code,
             context_ptr,
             context_length,
             scratch_buffer,
             response_length,
             NX_WAIT_FOREVER)))
    {
        printf("Command response failed!: error code = 0x%08x\r\n", status);
    }

    nx_azure_iot_json_writer_deinit(&json_writer);
}

static VOID sample_desired_properties_parse(NX_AZURE_IOT_PNP_CLIENT* pnp_client_ptr,
    NX_AZURE_IOT_JSON_READER* json_reader_ptr,
    UINT message_type,
    ULONG version)
{
    const UCHAR* component_ptr = NX_NULL;
    UINT component_len         = 0;
    NX_AZURE_IOT_JSON_READER name_value_reader;
    NX_AZURE_IOT_JSON_READER copy_json_reader = *json_reader_ptr;
    while (nx_azure_iot_pnp_client_desired_component_property_value_next(
               pnp_client_ptr, &copy_json_reader, message_type, &component_ptr, &component_len, &name_value_reader) ==
           NX_AZURE_IOT_SUCCESS)
    {
        if (pnp_vt_process_property_update(
                verified_telemetry_DB, pnp_client_ptr, component_ptr, component_len, &name_value_reader, version) ==
            NX_AZURE_IOT_SUCCESS)
        {
            printf("Verified Telemetry Property updated\r\n\n");
        }
        else
        {
            if (component_ptr)
            {
                printf("Component=%.*s is not implemented by the Device\r\n", component_len, component_ptr);
            }
            else
            {
                printf("Root component is not implemented by the Device\r\n");
            }
        }
    }
    copy_json_reader = *json_reader_ptr;
    while (nx_azure_iot_pnp_client_reported_component_property_value_next(
               pnp_client_ptr, &copy_json_reader, message_type, &component_ptr, &component_len, &name_value_reader) ==
           NX_AZURE_IOT_SUCCESS)
    {
        pnp_vt_process_reported_property_sync(
            verified_telemetry_DB, pnp_client_ptr, component_ptr, component_len, &name_value_reader, version);
    }

    pnp_vt_send_desired_property_after_boot(verified_telemetry_DB, pnp_client_ptr, message_type);
}

static VOID sample_device_desired_property_action(SAMPLE_CONTEXT* context)
{

    UINT status = 0;
    NX_AZURE_IOT_JSON_READER json_reader;
    ULONG desired_properties_version;

    if (context->state != SAMPLE_STATE_CONNECTED)
    {
        return;
    }

    if ((status = nx_azure_iot_pnp_client_desired_properties_receive(
             &(context->iotpnp_client), &json_reader, &desired_properties_version, NX_WAIT_FOREVER)))
    {
        printf("desired properties receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    printf("Received desired property");
    printf("\r\n");

    sample_desired_properties_parse(
        &(context->iotpnp_client), &json_reader, NX_AZURE_IOT_PNP_DESIRED_PROPERTIES, desired_properties_version);

    nx_azure_iot_json_reader_deinit(&json_reader);
}

static VOID sample_device_reported_property_action(SAMPLE_CONTEXT* context)
{
    UINT status;

    if (context->state != SAMPLE_STATE_CONNECTED)
    {
        return;
    }

    /* Only report once */
    if (sample_device_properties_sent == 0)
    {
        if ((status = sample_pnp_device_led_state_property(&sample_device, &(context->iotpnp_client))))
        {
            printf("Failed sample_pnp_device_led_state_property: error code = 0x%08x\r\n", status);
        }
        else
        {
            sample_device_properties_sent = 1;
        }
    }

    /* Only report when changed */
    if (sample_led_state_reported != sample_device.sensorLEDState)
    {
        if ((status = sample_pnp_device_led_state_property(&sample_device, &(context->iotpnp_client))))
        {
            printf("Failed sample_pnp_device_led_state_property: error code = 0x%08x\r\n", status);
        }
        else
        {
            sample_led_state_reported = sample_device.sensorLEDState;
        }
    }

    if ((status = pnp_vt_properties(verified_telemetry_DB, &(context->iotpnp_client))))
    {
        printf("Failed sample_pnp_vt_properties: error code = 0x%08x\r\n", status);
    }
}

static VOID sample_device_properties_get_action(SAMPLE_CONTEXT* context)
{
    UINT status = 0;
    NX_AZURE_IOT_JSON_READER json_reader;
    ULONG desired_properties_version;

    if (context->state != SAMPLE_STATE_CONNECTED)
    {
        return;
    }

    if ((status = nx_azure_iot_pnp_client_properties_receive(
             &(context->iotpnp_client), &json_reader, &desired_properties_version, NX_WAIT_FOREVER)))
    {
        printf("Get all properties receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    printf("Received all properties");
    printf("\r\n");

    sample_desired_properties_parse(
        &(context->iotpnp_client), &json_reader, NX_AZURE_IOT_PNP_PROPERTIES, desired_properties_version);

    nx_azure_iot_json_reader_deinit(&json_reader);
}

static VOID sample_telemetry_action(SAMPLE_CONTEXT* context)
{
    UINT status;

    if (context->state != SAMPLE_STATE_CONNECTED)
    {
        return;
    }

    if ((status = sample_pnp_device_telemetry_send(&sample_device, &(context->iotpnp_client))) != NX_AZURE_IOT_SUCCESS)
    {
        printf("Failed to send sample_pnp__telemetry_send, error: %d", status);
    }
}

#ifdef ENABLE_DPS
static UINT sample_dps_entry(NX_AZURE_IOT_PROVISIONING_CLIENT* prov_client_ptr,
    UCHAR** iothub_hostname,
    UINT* iothub_hostname_length,
    UCHAR** iothub_device_id,
    UINT* iothub_device_id_length)
{
    UINT status;

    /* Initialize IoT provisioning client.  */
    if ((status = nx_azure_iot_provisioning_client_initialize(prov_client_ptr,
             &nx_azure_iot,
             (UCHAR*)ENDPOINT,
             sizeof(ENDPOINT) - 1,
             (UCHAR*)ID_SCOPE,
             sizeof(ID_SCOPE) - 1,
             (UCHAR*)REGISTRATION_ID,
             sizeof(REGISTRATION_ID) - 1,
             _nx_azure_iot_tls_supported_crypto,
             _nx_azure_iot_tls_supported_crypto_size,
             _nx_azure_iot_tls_ciphersuite_map,
             _nx_azure_iot_tls_ciphersuite_map_size,
             nx_azure_iot_tls_metadata_buffer,
             sizeof(nx_azure_iot_tls_metadata_buffer),
             &root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_initialize!: error code = 0x%08x\r\n", status);
        return (status);
    }

    /* Initialize length of hostname and device ID. */
    *iothub_hostname_length  = sizeof(sample_iothub_hostname);
    *iothub_device_id_length = sizeof(sample_iothub_device_id);

#if (USE_DEVICE_CERTIFICATE == 1)

    /* Initialize the device certificate.  */
    if ((status = nx_secure_x509_certificate_initialize(&device_certificate,
             (UCHAR*)sample_device_cert_ptr,
             (USHORT)sample_device_cert_len,
             NX_NULL,
             0,
             (UCHAR*)sample_device_private_key_ptr,
             (USHORT)sample_device_private_key_len,
             DEVICE_KEY_TYPE)))
    {
        printf("Failed on nx_secure_x509_certificate_initialize!: error code = 0x%08x\r\n", status);
    }

    /* Set device certificate.  */
    else if ((status = nx_azure_iot_provisioning_client_device_cert_set(prov_client_ptr, &device_certificate)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_device_cert_set!: error code = 0x%08x\r\n", status);
    }
#else

    /* Set symmetric key.  */
    if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(
             prov_client_ptr, (UCHAR*)DEVICE_SYMMETRIC_KEY, sizeof(DEVICE_SYMMETRIC_KEY) - 1)))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!: error code = 0x%08x\r\n", status);
    }
#endif /* USE_DEVICE_CERTIFICATE */

    /* Register device */
    else if ((status = nx_azure_iot_provisioning_client_register(prov_client_ptr, NX_WAIT_FOREVER)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_register!: error code = 0x%08x\r\n", status);
    }

    /* Get Device info */
    else if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(prov_client_ptr,
                  sample_iothub_hostname,
                  iothub_hostname_length,
                  sample_iothub_device_id,
                  iothub_device_id_length)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_iothub_device_info_get!: error code = 0x%08x\r\n", status);
    }
    else
    {
        *iothub_hostname  = sample_iothub_hostname;
        *iothub_device_id = sample_iothub_device_id;
    }

    /* Destroy Provisioning Client.  */
    nx_azure_iot_provisioning_client_deinitialize(prov_client_ptr);

    return (status);
}
#endif /* ENABLE_DPS */

/**
 *
 * Sample Event loop
 *
 *
 *       +--------------+           +--------------+      +--------------+       +--------------+
 *       |              |  INIT     |              |      |              |       |              |
 *       |              | SUCCESS   |              |      |              |       |              +--------+
 *       |    INIT      |           |    CONNECT   |      |  CONNECTING  |       |   CONNECTED  |        | (TELEMETRY |
 *       |              +----------->              +----->+              +------->              |        |  METHOD |
 *       |              |           |              |      |              |       |              <--------+  DEVICETWIN)
 *       |              |           |              |      |              |       |              |
 *       +-----+--------+           +----+---+-----+      +------+-------+       +--------+-----+
 *             ^                         ^   |                   |                        |
 *             |                         |   |                   |                        |
 *             |                         |   |                   |                        |
 *             |                         |   | CONNECT           | CONNECTING             |
 *             |                         |   |  FAIL             |   FAIL                 |
 * REINITIALIZE|                RECONNECT|   |                   |                        |
 *             |                         |   |                   v                        |  DISCONNECT
 *             |                         |   |        +----------+-+                      |
 *             |                         |   |        |            |                      |
 *             |                         |   +------->+            |                      |
 *             |                         |            | DISCONNECT |                      |
 *             |                         |            |            +<---------------------+
 *             |                         +------------+            |
 *             +--------------------------------------+            |
 *                                                    +------------+
 *
 *
 *
 */
static VOID sample_event_loop(SAMPLE_CONTEXT* context)
{
    ULONG app_events;
    UINT loop = NX_TRUE;

    while (loop)
    {
        /* Pickup IP event flags.  */
        if (tx_event_flags_get(
                &(context->sample_events), SAMPLE_ALL_EVENTS, TX_OR_CLEAR, &app_events, 5 * NX_IP_PERIODIC_RATE))
        {
            if (context->state == SAMPLE_STATE_CONNECTED)
            {
                sample_trigger_action(context);
            }

            continue;
        }

        if (app_events & SAMPLE_CONNECT_EVENT)
        {
            sample_connect_action(context);
        }

        if (app_events & SAMPLE_INITIALIZATION_EVENT)
        {
            sample_initialize_iothub(context);
        }

        if (app_events & SAMPLE_DEVICE_PROPERTIES_GET_EVENT)
        {
            sample_device_properties_get_action(context);
        }

        if (app_events & SAMPLE_COMMAND_MESSAGE_EVENT)
        {
            sample_command_action(context);
        }

        if (app_events & SAMPLE_DEVICE_DESIRED_PROPERTIES_EVENT)
        {
            sample_device_desired_property_action(context);
        }

        if (app_events & SAMPLE_TELEMETRY_SEND_EVENT)
        {
            sample_telemetry_action(context);
        }

        if (app_events & SAMPLE_DEVICE_REPORTED_PROPERTIES_EVENT)
        {
            sample_device_reported_property_action(context);
        }

        if (app_events & SAMPLE_DISCONNECT_EVENT)
        {
            sample_disconnect_action(context);
        }

        if (app_events & SAMPLE_CONNECTED_EVENT)
        {
            sample_connected_action(context);
        }

        if (app_events & SAMPLE_RECONNECT_EVENT)
        {
            sample_connection_error_recover(context);
        }

        sample_trigger_action(context);
    }
}

static VOID sample_context_init(SAMPLE_CONTEXT* context)
{
    memset(context, 0, sizeof(SAMPLE_CONTEXT));
    tx_event_flags_create(&(context->sample_events), (CHAR*)"sample_app");
}

static VOID log_callback(az_log_classification classification, UCHAR* msg, UINT msg_len)
{
    if (classification == AZ_LOG_IOT_AZURERTOS)
    {
        printf("%.*s", msg_len, (CHAR*)msg);
    }
}

static UINT sample_components_init()
{
    UINT status;

    verified_telemetry_DB = sample_pnp_verified_telemetry_user_init();

    if ((status = sample_pnp_device_init(&sample_device,
             (UCHAR*)sample_device_component,
             sizeof(sample_device_component) - 1,
             SAMPLE_DEFAULT_DEVICE_SENSOR_READING,
             verified_telemetry_DB)))
    {
        printf("Failed to initialize %s: error code = 0x%08x\r\n", sample_device_component, status);
    }

    sample_led_state_reported     = 0;
    sample_device_properties_sent = 0;

    return (status);
}

VOID sample_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time))
{
    UINT status;
    nx_azure_iot_log_init(log_callback);

    if ((status = sample_components_init()))
    {
        printf("Failed on initialize sample components!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Create Azure IoT handler.  */
    if ((status = nx_azure_iot_create(&nx_azure_iot,
             (UCHAR*)"Azure IoT",
             ip_ptr,
             pool_ptr,
             dns_ptr,
             nx_azure_iot_thread_stack,
             sizeof(nx_azure_iot_thread_stack),
             NX_AZURE_IOT_THREAD_PRIORITY,
             unix_time_callback)))
    {
        printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Initialize CA certificate. */
    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert,
             (UCHAR*)azure_iot_root_ca,
             (USHORT)azure_iot_root_ca_len,
             NX_NULL,
             0,
             NULL,
             0,
             NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return;
    }

    sample_context_init(&sample_context);

    sample_context.state = SAMPLE_STATE_INIT;
    tx_event_flags_set(&(sample_context.sample_events), SAMPLE_INITIALIZATION_EVENT, TX_OR);

    /* Handle event loop */
    sample_event_loop(&sample_context);

    nx_azure_iot_delete(&nx_azure_iot);
}
