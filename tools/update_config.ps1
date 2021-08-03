# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

param(
      [Parameter(Mandatory=$true)]
      [string]$ConfigFilePath
     )

if (-not (Test-Path -Path $ConfigFilePath -PathType Leaf))
{
    throw "$ConfigFilePath doesn't exist or not accessible" 
}

$updatedContent = Get-Content $ConfigFilePath | 
    ForEach-Object {
        if($_ -like "//#define ENABLE_DPS*"){
            return "#define ENABLE_DPS"
        }

        if($_ -like "#define DEVICE_SYMMETRIC_KEY*"){
            return "#define DEVICE_SYMMETRIC_KEY `"$env:PRIKEY`""
        }

        if($_ -like "#define REGISTRATION_ID*"){
            return "#define REGISTRATION_ID `"$env:REGISTRATIONID`""
        }

        if($_ -like "#define ID_SCOPE*"){
            return "#define ID_SCOPE `"$env:SCOPEID`""
        }

        if($_ -like "#define WIFI_SSID*"){
            return "#define WIFI_SSID `"MSFTGUEST`""
        }

        if($_ -like "#define WIFI_MODE*"){
            return "#define WIFI_MODE None"
        }

        return $_
    }
Set-Content -Path $ConfigFilePath -Value $updatedContent

