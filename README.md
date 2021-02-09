![](https://github.com/Azure/Verified-Telemetry-Device-Sample/workflows/Markdown%20links/badge.svg)
![](https://github.com/Azure/Verified-Telemetry-Device-Sample/workflows/AZ3166/badge.svg)
![](https://github.com/Azure/Verified-Telemetry-Device-Sample/workflows/STM32L4_L4+/badge.svg)

# Getting Started with Verified Telemetry, Azure RTOS and Azure IoT

## Getting Started Guides

These Getting Started guides shows device developers how to include Verified Telemetry with [Azure IoT](https://azure.microsoft.com/overview/iot/) and [Azure RTOS](https://docs.microsoft.com/azure/rtos/).

* MXCHIP: 
  * [AZ3166](MXChip/AZ3166)
* STMicroelectronics:
  * [B-L475E-IOT01A](STMicroelectronics/STM32L4_L4+)
  * [B-L4S5I-IOT01A](STMicroelectronics/STM32L4_L4+)

## Verified Telemetry
Verified Telemetry is a state-of-the-art solution to determine the functioning state of a device sensor, working or faulty.

This is achieved by collecting an intelligent fingerprint for the sensor, and measuring future variance from this fingerprint to imply the sensors status.

The fingerprint captures electrical properties of the sensor, such as voltage and current, using the Verified Telemetry Device SDK integrated directly on the IoT device.

|Component |Description |
|-|-|
|[Verified Telemetry Device SDK](https://github.com/Azure/Verified-Telemetry) |The SDK which builds on the Azure RTOS middleware |
|[Verified Telemetry Device Sample](https://github.com/Azure/Verified-Telemetry-Device-Sample) |These Getting Started guides shows device developers how to combine Verified Telemetry with [Azure IoT](https://azure.microsoft.com/overview/iot/) and [Azure RTOS](https://docs.microsoft.com/azure/rtos/). |
|[Verified Telemetry Solution Sample](https://github.com/Azure/Verified-Telemetry-Solution-Sample) | Uses InfluxDB, Grafana and the [Azure IoT Node.js SDK](https://github.com/Azure/azure-iot-sdk-node) to communicate with [Azure IoT Hub](https://docs.microsoft.com/azure/iot-hub/) and showcase how the Verified Telemetry features can be utilized in real world scenarios.|
