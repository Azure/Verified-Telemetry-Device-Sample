# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/startup/MXChip_AZ3166.ld")

set(SOURCES
    startup/system_stm32f4xx.c
    startup/startup_stm32f412rx.s
    startup/tx_initialize_low_level.S
    stm32cubef4/stm32f4xx_hal_msp.c
    azure_config.h
    board_init.c
    console.c
    screen.c
    main.c
    wwd_networking.c

    sample_pnp_iot_device.c
    sample_pnp_device_component.c
    sample_nx_verified_telemetry_init.c
    sample_vt_device_driver.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    azrtos::threadx
    azrtos::netxduo
    az::iot::vt

    stm32cubef4
    wiced_sdk
    app_common
)

target_link_options(${PROJECT_NAME}
    PUBLIC 
        -T${LINKER_SCRIPT} -Wl,-Map=${PROJECT_NAME}.map)

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        LINK_DEPENDS ${LINKER_SCRIPT}
        SUFFIX ".elf"
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

create_bin_output(${PROJECT_NAME})
firmware_size(${PROJECT_NAME})