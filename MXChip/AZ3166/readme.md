# Getting started with the MXChip AZ3166 IoT DevKit for Verified Telemetry

## Prerequisites

* A PC running Microsoft Windows (Windows 10 recommended)
* If you don't have an Azure subscription, [create one for free](https://azure.microsoft.com/free/?WT.mc_id=A261C142F) before you begin.
* Hardware

    > * The [MXChip AZ3166 IoT DevKit](https://aka.ms/iot-devkit) (MXChip DevKit)
    > * Wi-Fi 2.4 GHz
    > * USB 2.0 A male to Micro USB male cable
    > * [MXChip Edge Connector](https://www.sparkfun.com/products/13989)
    > * [Analog Accelerometer ADXL335](https://www.sparkfun.com/products/9269)
    > * [Soil Moisture Sensor](https://www.dfrobot.com/product-1385.html)

## Prepare the development environment

To set up your development environment, first you clone a GitHub repo that contains all the assets you need for the tutorial. Then you install a set of programming tools.

### Clone the repo for the tutorial

Clone the following repo to download all sample device code, setup scripts, and offline versions of the documentation.

To clone the repo, run the following command:

```shell
git clone --recursive https://github.com/Azure/Verified-Telemetry-Device-Sample.git
```


### Install the tools

The cloned repo contains a setup script that installs and configures the required tools. If you installed these tools in another tutorial in the getting started guide, you don't need to do it again.

> Note: The setup script installs the following tools:
> * [GCC](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm): Compile
> * [CMake](https://cmake.org): Build
> * [Termite](https://www.compuphase.com/software_termite.htm): Monitor COM port output for connected devices
> * [Azure IoT Explorer](https://github.com/Azure/azure-iot-explorer/releases): Cross-platform utility to  monitor and manage Azure IoT resources

To run the setup script:

1. From File Explorer, navigate to the following path in the repo and run the setup script named *get-toolchain.bat*:

    > *getting-started\tools\get-toolchain.bat*

    After the installation completes, the Azure IoT Explorer opens automatically. Keep the IoT Explorer open, you'll use it in later steps.

1. After the installation, open a new console window to recognize the configuration changes made by the setup script. Use this console to complete the remaining programming tasks in the tutorial. You can use Windows CMD, PowerShell, or Git Bash for Windows.
1. Run the following code to confirm that CMake version 3.14 or later is installed.

    ```shell
    cmake --version
    ```

## Prepare Azure resources

To prepare Azure cloud resources and connect a device to Azure, you can use Azure CLI. There are two ways to access the Azure CLI: by using the Azure Cloud Shell, or by installing Azure CLI locally.  Azure Cloud Shell lets you run the CLI in a browser so you don't have to install anything.

Use one of the following options to run Azure CLI.  

If you prefer to run Azure CLI locally:

1. If you already have Azure CLI installed locally, run `az --version` to check the version. This tutorial requires Azure CLI 2.10.1 or later.
1. To install or upgrade, see [Install Azure CLI](https://docs.microsoft.com/cli/azure/install-azure-cli?view=azure-cli-latest). If you install Azure CLI locally, you can run CLI commands in the GCC Command Prompt, Git Bash for Windows, or PowerShell.

If you prefer to run Azure CLI in the browser-based Azure Cloud Shell:

1. Use your Azure account credentials to sign into the Azure Cloud shell at https://shell.azure.com/.
    > Note: If this is the first time you've used the Cloud Shell, it prompts you to create storage, which is required to use the Cloud Shell.  Select a subscription to create a storage account and Microsoft Azure Files share.
1. Select Bash or PowerShell as your preferred CLI environment in the **Select environment** dropdown. If you plan to use Azure Cloud Shell, keep your browser open to run the Azure CLI commands in this tutorial.

    ![Select CLI environment](media/cloud-shell-environment.png)

### Create an IoT hub

You can use Azure CLI to create an IoT hub that handles events and messaging for your device.

To create an IoT hub:

1. In your CLI console, run the [az extension add](https://docs.microsoft.com/cli/azure/extension?view=azure-cli-latest#az-extension-add) command to add the Microsoft Azure IoT Extension for Azure CLI to your CLI shell. The IOT Extension adds IoT Hub, IoT Edge, and IoT Device Provisioning Service (DPS) specific commands to Azure CLI.

   ```shell
   az extension add --name azure-iot
   ```

1. Run the [az group create](https://docs.microsoft.com/cli/azure/group?view=azure-cli-latest#az-group-create) command to create a resource group. The following command creates a resource group named *MyResourceGroup* in the *centralus* region.

    > Note: You can optionally set an alternate `location`. To see available locations, run [az account list-locations](https://docs.microsoft.com/cli/azure/account?view=azure-cli-latest#az-account-list-locations). For this tutorial we recommend using `centralus` as in the example CLI command. The IoT Plug and Play feature that you use later in the tutorial, is currently only available in three regions, including `centralus`.

    ```shell
    az group create --name MyResourceGroup --location centralus
    ```

1. Run the [az iot hub create](https://docs.microsoft.com/cli/azure/iot/hub?view=azure-cli-latest#az-iot-hub-create) command to create an IoT hub. It might take a few minutes to create an IoT hub.

    *YourIotHubName*. Replace this placeholder below with the name you chose for your IoT hub. An IoT hub name must be globally unique in Azure. This placeholder is used in the rest of this tutorial to represent your unique IoT hub name.

    ```shell
    az iot hub create --resource-group MyResourceGroup --name {YourIoTHubName}
    ```

1. After the IoT hub is created, view the JSON output in the console, and copy the `hostName` value to a safe place. You use this value in a later step. The `hostName` value looks like the following example:

    `{Your IoT hub name}.azure-devices.net`

### Register a device

In this section, you create a new device instance and register it with the IoT hub you created. You will use the connection information for the newly registered device to securely connect your physical device in a later section.

To register a device:

1. In your console, run the [az iot hub device-identity create](https://docs.microsoft.com/cli/azure/ext/azure-cli-iot-ext/iot/hub/device-identity?view=azure-cli-latest#ext-azure-cli-iot-ext-az-iot-hub-device-identity-create) command. This creates the simulated device identity.

    *YourIotHubName*. Replace this placeholder below with the name you chose for your IoT hub.

    *MyMXChipDevice*. You can use this name directly for the device in CLI commands in this tutorial. Optionally, use a different name.

    ```shell
    az iot hub device-identity create --device-id MyMXChipDevice --hub-name {YourIoTHubName}
    ```

1. After the device is created, view the JSON output in the console, and copy the `deviceId` and `primaryKey` values to use in a later step.

Confirm that you have the copied the following values from the JSON output to use in the next section:

> * `hostName`
> * `deviceId`
> * `primaryKey`

## Prepare the device

To connect the MXChip DevKit to Azure, you'll modify a configuration file for Wi-Fi and Azure IoT settings, rebuild the image, and flash the image to the device.

### Add configuration

1. In a text editor, edit the file *getting-started\MXChip\AZ3166\app\azure_config.h* to set the Wi-Fi constants to the following values from your local environment.

    |Constant name|Value|
    |-------------|-----|
    |`WIFI_SSID` |{*Your Wi-Fi ssid*}|
    |`WIFI_PASSWORD` |{*Your Wi-Fi password*}|
    |`WIFI_MODE` |{*Your Wi-Fi security type*}|

1. Edit the same file to set the Azure IoT device information constants to the values that you saved after you created Azure resources.

    |Constant name|Value|
    |-------------|-----|
    |`IOT_HUB_HOSTNAME` |{*Your Iot hub hostName value*}|
    |`IOT_DEVICE_ID` |{*Your deviceID value*}|
    |`IOT_PRIMARY_KEY` |{*Your primaryKey value*}|

### Build the image

In your console or in File Explorer, run the script *rebuild.bat* at the following path to build the image:

> *getting-started\MXChip\AZ3166\tools\rebuild.bat*

After the build completes, confirm that the binary files were created in the following path:

> *getting-started\MXChip/AZ3166\build\app\mxchip_azure_iot.bin*

### Flash the image

1. On the MXChip DevKit, locate the **Reset** button, and the Micro USB port. You use these components in the following steps. Both are highlighted in the following picture:

    ![MXChip DevKit reset button and micro usb port](media/mxchip-iot-devkit.png)

1. Connect the Micro USB cable to the Micro USB port on the MXChip DevKit, and then connect it to your computer.

1. In File Explorer, find the MXChip DevKit device connected to your computer. It is a driver labeled as **AZ3166**.

1. Copy the image file *mxchip_azure_iot.bin* that you created in the previous section, and paste it into the root folder of the MXChip DevKit. The flashing process starts automatically.

    > Note: During the flashing process, the RED LED toggled on MXChip DevKit. The process completes in a few seconds without further notification.

### Connect Sparkfun Edge Connector which is available [here](https://www.sparkfun.com/products/13989)     ***Verified Telemetry***

### Connect Sensors         ***Verified Telemetry***

Refer to the table below to connect the two sensors: [ADXL335](https://www.sparkfun.com/products/9269) & [Soil Moisture](https://www.dfrobot.com/product-1385.html)

| Sensor Name   | Sensor Pin           | Sparkfun Edge Connector Pin | MXChip Pin |
|---------------|----------------------|-----------------------------|------------|
| ADXL335       | X Axis analog output | 4                           | PA5        |
| ADXL335       | VCC                  | 8                           | PC13       |
| Soil Moisture | Analog Out           | 0                           | PB0        |
| Soil Moisture | VCC                  | 12                          | PB2        |

### Confirm device connection details

You can use the **Termite** utility to monitor communication and confirm that your device is set up correctly.
> Note: If you have issues getting your device to initialize or connect after flashing, see [Troubleshooting](../../docs/troubleshooting.md).

1. Open Device Manager and find the COM port for the MXChip IoT DevKit.

    ![COM Port](./media/com_port.png)

1. Start **Termite**.
1. Select **Settings**.
1. In the **Serial port settings** dialog, check the following settings and update if needed:

    * **Baud rate**: 115,200
    * **Data bits**: 8
    * **Stop bits**: 1

    ![Termite](./media/termite-settings.png)

1. Select OK.

    Now you can view the terminal output. The MXChip DevKit provides initialization messages about your connection and key protocols, and then publishes telemetry from the sensors on the device.

    ![Termite](./media/termite.png)

## Setup Azure IoT Hub details in Azure IoT Explorer


To add a connection to your IoT hub:

1. In your CLI console, run the [az iot hub show-connection-string](https://docs.microsoft.com/en-us/cli/azure/iot/hub?view=azure-cli-latest#az-iot-hub-show-connection-string) command to get the connection string for your IoT hub.

    ```shell
    az iot hub show-connection-string --name {YourIoTHubName}
    ```

1. Copy the connection string without the surrounding quotation characters.
1. In Azure IoT Explorer, select **IoT hubs > Add connection**.
1. Paste the connection string into the **Connection string** box.
1. Select **Save**.

    ![Azure IoT Explorer connection string](media/azure-iot-explorer-create-connection.png)

If the connection succeeds, the Azure IoT Explorer switches to a **Devices** view and lists your device.

## Add PnP Model Files  ***Verified Telemetry***

To add a device model to IoT Explorer:

1. Navigate to **Home**.
1. Select **IoT Plug and Play Settings**.
1. Select **Add > Local folder**.
1. Select **Pick a folder**.
1. Browse to */verified-telemetry-device-sample/core/model/*. The folder contains the Verified Telemetry Interface and sample model files.
1. Click **Select**.
1. Select **Save**.

    ![Azure IoT Explorer add device model](media/azure-iot-explorer-PnPModelPath.png)

## Access the IoT Plug and Play components ***Verified Telemetry***

To access the IoT Plug and Play components on your device:

1. Select **IoT hubs > View devices in this hub**.
1. Select your device.
1. Select **IoT Plug and Play components**.

    ![Azure IoT Explorer Plug and Play components](media/azure-iot-explorer-Components.png)

## Sample Device component  ***Verified Telemetry***

The [Device Sensors Interface](../../core/model/sample/device.json) contains telemetries, properties and commands for the demo device.

| Tab | Type | Name | Description |
|---|---|---|---|
| **Interface** | Interface | `Sample Device` | Example standard Device Model for getting started with Verified Telemetry/Dependable IoT Guide |
| **Properties (read-only)** | Property | `ledState` | The current state of the LED |
| **Commands** | Command | `setLedState` | Enable or disable the LED |
| **Telemetry** | Telemetry | `soilMoistureExternal`, `accelerometerXExternal`, `temperature`, `pressure`, `humidityPercentage`, `acceleration`, `magnetic` | Telemetries originating from various onboard and external sensors |

### View Telemetry from Device component ***Verified Telemetry***

To view telemetry from Sample Device component:

1. Select the **Telemetry** tab.
1. Select **Start**.  IoT Explorer displays telemetry using the interval set in device code.

    ```json
    6:50:36 PM, 11/20/2020:
    {
    "body": {
        "soilMoistureExternal": 312,
        "accelerometerXExternal": 1981,
        "temperature": 30.93,
        "pressure": 952.8,
        "humidityPercentage": 88.17,
        "acceleration": -5.06,
        "magnetic": 12
    },
    "enqueuedTime": "2020-11-20T13:20:36.189Z",
    "properties": {}
    }
    6:50:29 PM, 11/20/2020:
    {
    "body": {
        "soilMoistureExternal": 313,
        "accelerometerXExternal": 1977,
        "temperature": 30.92,
        "pressure": 952.76,
        "humidityPercentage": 88.19,
        "acceleration": -5,
        "magnetic": 25.5
    },
    "enqueuedTime": "2020-11-20T13:20:29.281Z",
    "properties": {}
    }
    ```
## Verified Telemetry Information Interface ***Verified Telemetry***
The main interface of Verified Telemetry is [Verified Telemetry Information](../../core/model/vTInfo.json).

| Type | Name | Description |
|---|---|---|
| **Properties (read-only)** | `telemetryStatus` | Status of the telemetry, i.e. Working/Faulty to which the component of this interface is asscoiated. |
| **Properties (read-only)** | `fingerprintType` | Type of the fingerprint (String). e.g., FallCurve or CurrentSense or Custom. |
| **Properties (read-only)** | `fingerprintTemplate` | Template Fingerprint information in CSV string. |
| **Commands** | `resetFingerprintTemplate` | This command will reset the template fingerprint |

##Steps to setup Verified Telemetry components vTsoilMoistureExternal and vTaccelerometerXExternal ***Verified Telemetry***

* Ensure `enableVerifiedTelemetry` from the root/default component is true

    ![Setting enableVerifiedTelemetry true ](media/azure-iot-explorer-enableVT.png)

* Issue command `resetFingerprintTemplate` in the vTsoilMoistureExternal component for setting up Verified Telemetry for the telemetry named 'soilMoistureExternal'

    ![Issue command to setup VT for telemetry soilMoistureExternal](media/azure-iot-explorer-resetSM.png)

* Issue command `resetFingerprintTemplate` in the vTaccelerometerXExternal component for setting up Verified Telemetry for the telemetry named 'accelerometerXExternal'

    ![Issue command to setup VT for telemetry accelerometerXExternal](media/azure-iot-explorer-resetAcc.png)

## Consuming Verified Telemetry Information  ***Verified Telemetry***
* The property `deviceStatus` from the root/default component indicates whether all the telemetries supported by Verified Telemetry are working or not

    ![Checking deviceStatus ](media/azure-iot-explorer-OverallStatus.png)

* The property `telemetryStatus` in the vTsoilMoistureExternal component indicates whether the telemetry'soilMoistureExternal' is correct or not

    ![Checking telemetryStatus for telemetry soilMoistureExternal](media/azure-iot-explorer-SMStatus.png)

* The property `telemetryStatus` in the vTaccelerometerXExternal component indicates whether the telemetry'accelerometerXExternal' is correct or not

    ![Checking telemetryStatus for telemetry accelerometerXExternal](media/azure-iot-explorer-AccStatus.png)

* In case of fault with the Soil Moisture sensor, the property `telemetryStatus` in the vTsoilMoistureExternal component would be false indicating that the telemetry'soilMoistureExternal' is NOT correct and should not be consumed by upstream processes
   
    ![Fault in telemetry soilMoistureExternal](media/azure-iot-explorer-SMStatusFault.png)

## Next Steps ***Verified Telemetry***
* With this sample, you have now setup Verified Telemetry/Dependable IoT on MXChip with Azure RTOS and Azure IoT
* Refer to our [sample Solution Template](https://dev.azure.com/DependableIoT/DependableIoT-AzureIoT/_git/verified-telemetry-solution-sample) to understand how the additional data from Verified Telemetry can help to increase data quality
* For further steps to include Verified Telemetry in your existing solution, refer to documentation in the [Verified Telemetry Library](https://dev.azure.com/DependableIoT/DependableIoT-AzureIoT/_git/verified-telemetry)
