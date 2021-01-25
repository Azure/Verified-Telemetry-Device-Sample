# Getting Started with Verified Telemetry with Azure RTOS and Azure IoT

These Getting Started guides shows device developers how to include Verified Telemetry with Azure IoT on Azure RTOS. 

* MXCHIP: 
  * [AZ3166](MXChip/AZ3166)
* STMicroelectronics: 
  * [B-L475E-IOT01A](STMicroelectronics/STM32L4_L4+)
  * [B-L4S5I-IOT01A](STMicroelectronics/STM32L4_L4+)

# [Verified Telemetry Library](https://github.com/Azure/Verified-Telemetry)
- Verified Telemetry is a state-of-the-art solution to determine the working of the sensor, i.e., working or faulty, consequently used to determine the quality of the sensed data. 
- This is achieved by devising an intelligent fingerprint for a sensor to determine the status of the sensor.  
- The sensor fingerprint captures electrical properties of the sensor such as voltage and current using seamless software code running on the IoT device. 

# Solution Template
We have built a [sample solution template](https://github.com/Azure/Verified-Telemetry-Solution-Sample) which uses Grafana, InfluxDB and a Node.js backend to communicate with Azure IoT Hub and showcase how the Verified Telemetry features can be utilised in real world scenarios.

# PnP Model Files for Verified Telemetry
PnP model files of common Verified Telemetry like [Verified Telemetry Information Interface](core/model/vTInfo.json) would be published soon in the Azure PnP model repo.
