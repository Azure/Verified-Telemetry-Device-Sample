# Install script for directory: C:/Users/v-sum/Desktop/vT_PrivateBeta/V/core/lib/netxduo

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/mxchip_azure_iot")
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
  include("C:/Users/v-sum/Desktop/vT_PrivateBeta/V/MXChip/AZ3166/build/lib/netxduo/ports/cortex_m4/gnu/cmake_install.cmake")
  include("C:/Users/v-sum/Desktop/vT_PrivateBeta/V/MXChip/AZ3166/build/lib/netxduo/common/cmake_install.cmake")
  include("C:/Users/v-sum/Desktop/vT_PrivateBeta/V/MXChip/AZ3166/build/lib/netxduo/addons/cmake_install.cmake")
  include("C:/Users/v-sum/Desktop/vT_PrivateBeta/V/MXChip/AZ3166/build/lib/netxduo/crypto_libraries/cmake_install.cmake")
  include("C:/Users/v-sum/Desktop/vT_PrivateBeta/V/MXChip/AZ3166/build/lib/netxduo/nx_secure/cmake_install.cmake")

endif()

