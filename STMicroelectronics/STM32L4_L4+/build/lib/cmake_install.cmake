# Install script for directory: C:/Repos/Github-Verified-Telemetry-Device-Sample/STMicroelectronics/STM32L4_L4+/lib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/stm32l4_azure_iot")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Repos/Github-Verified-Telemetry-Device-Sample/STMicroelectronics/STM32L4_L4+/build/lib/threadx/cmake_install.cmake")
  include("C:/Repos/Github-Verified-Telemetry-Device-Sample/STMicroelectronics/STM32L4_L4+/build/lib/jsmn/cmake_install.cmake")
  include("C:/Repos/Github-Verified-Telemetry-Device-Sample/STMicroelectronics/STM32L4_L4+/build/lib/verified_telemetry/cmake_install.cmake")
  include("C:/Repos/Github-Verified-Telemetry-Device-Sample/STMicroelectronics/STM32L4_L4+/build/lib/netxduo/cmake_install.cmake")
  include("C:/Repos/Github-Verified-Telemetry-Device-Sample/STMicroelectronics/STM32L4_L4+/build/lib/stm32cubel4/cmake_install.cmake")
  include("C:/Repos/Github-Verified-Telemetry-Device-Sample/STMicroelectronics/STM32L4_L4+/build/lib/netx_driver/cmake_install.cmake")

endif()

