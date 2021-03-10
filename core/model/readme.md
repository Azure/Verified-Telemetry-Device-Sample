# Model File Description
## Verified Telemetry Information Interface
The main interface of Verified Telemetry is [Verified Telemetry Information](./vTInfo.json).

| Type | Name | Description |
|---|---|---|
| **Properties (read-only)** | `telemetryStatus` | Status of the telemetry, i.e. Working/Faulty to which the component of this interface is asscoiated. |
| **Properties (read-only)** | `fingerprintType` | Type of the fingerprint (String). e.g., FallCurve or CurrentSense or Custom. |
| **Properties (read-only)** | `fingerprintTemplate` | Template Fingerprint information in a Map |
| **Properties (read-only)** | `fingerprintTemplateConfidenceMetric` | Stores information on how much the Fingerprint Template can be trusted |
| **Commands** | `setResetFingerprintTemplate` | This command will reset the template fingerprint |
| **Commands** | `retrainFingerprintTemplate` | This command will retrain the template fingerprint |

## Verified Telemetry Device Interface
The [Verified Telemetry Device Information](./vTDevice.json) Interface implements and conveys device wide Verified Telemetry Settings and Information 
| Type | Name | Description |
|---|---|---|
| **Properties (writable)** | `enableVerifiedTelemetry` | Controls whether Fingerprint Collection and Evaluation is implemented or not. When this property is set to 'false', Telemetry Verification cannot be performed.  |
| **Properties (read-only)** | `deviceStatus` | Device status is set to false if any sensor supported by VT has a fault. |

## [Getting started guide](./gsg.json) interface has the following components:
1. Device Component using the [Device Sensors](./device.json) interface. This represents the Default Device Component from the ODMs.
1. vTDevice Component uses the [Verified Telemetry Device Information](./vTDevice.json) interface.
1. vTsoilMoistureExternal1 Component using the [Verified Telemetry Information](./vTInfo.json). This represents the verified telemetry component for soilMoistureExternal1 telemetry.
1. vTsoilMoistureExternal2 Component using the [Verified Telemetry Information](./vTInfo.json). This represents the verified telemetry component for soilMoistureExternal2 telemetry.

| Type | Name | Description | Interface ID |
|---|---|---|---|
| **Component** | `sampleDevice` | Default Device Component from the manufacturer. | dtmi:azure:verifiedtelemetry:sample:GSG;1 |
| **Component** | `vTDevice` | Device Level Verified Telemetry component | dtmi:azure:verifiedtelemetry:deviceinformation;1 | 
| **Component** | `vTsoilMoistureExternal1` | The Verified Telemetry component for soilMoistureExternal1 telemetry. | dtmi:azure:verifiedtelemetry:telemetryinformation;1 | 
| **Component** | `vTsoilMoistureExternal2` | The Verified Telemetry component for soilMoistureExternal2 telemetry. | dtmi:azure:verifiedtelemetry:telemetryinformation;1 | 