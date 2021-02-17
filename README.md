![](https://github.com/Azure/Verified-Telemetry-Device-Sample/workflows/Markdown%20links/badge.svg)
![](https://github.com/Azure/Verified-Telemetry-Device-Sample/workflows/AZ3166/badge.svg)
![](https://github.com/Azure/Verified-Telemetry-Device-Sample/workflows/STM32L4_L4+/badge.svg)

# Getting Started with Verified Telemetry, Azure RTOS and Azure IoT

## Verified Telemetry
Verified Telemetry (VT) is a state-of-the-art solution to determine the health of the sensor, i.e., working or faulty, which is consequently used to determine the quality of the sensed data. This is achieved by devising an intelligent “sensor fingerprint”, a set of unique electrical characteristics that differs between working and faulty sensors. The fingerprints can detect faults for a wide variety of off-the-shelf sensors and can be easily implemented with lightweight software code running on the IoT device.


| |Description |
|-|-|
|[Verified Telemetry Device SDK](https://github.com/Azure/Verified-Telemetry) |The SDK which builds on the Azure RTOS middleware |
|[Verified Telemetry Device Sample](https://github.com/Azure/Verified-Telemetry-Device-Sample) |These Getting Started guides shows device developers how to combine Verified Telemetry with [Azure IoT](https://azure.microsoft.com/overview/iot/) and [Azure RTOS](https://docs.microsoft.com/azure/rtos/). |
|[Verified Telemetry Solution Sample](https://github.com/Azure/Verified-Telemetry-Solution-Sample) | Uses InfluxDB, Grafana and the [Azure IoT Node.js SDK](https://github.com/Azure/azure-iot-sdk-node) to communicate with [Azure IoT Hub](https://docs.microsoft.com/azure/iot-hub/) and showcase how the Verified Telemetry features can be utilized in real world scenarios.|

## Getting Started Guides

The Getting Started guides shows device developers how to include Verified Telemetry with [Azure IoT](https://azure.microsoft.com/overview/iot/) and [Azure RTOS](https://docs.microsoft.com/azure/rtos/). Please find the following board specific guides:

* MXCHIP: 
  * [AZ3166](MXChip/AZ3166)
* STMicroelectronics:
  * [B-L475E-IOT01A](STMicroelectronics/STM32L4_L4+)
  * [B-L4S5I-IOT01A](STMicroelectronics/STM32L4_L4+)

  > Note: Verified Telemetry status is supported only to analog sensors. We are currently working on extending the SDK to support digital sensors. 

## License
The Azure Verified Telemetry Getting Started guides are licensed under the [MIT](./LICENSE.txt) license. 
