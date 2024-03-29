# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/system_stm32l4xx.c
    startup/tx_initialize_low_level.S
    stm32cubel4/stm32l4xx_hal_msp.c
    stm32cubel4/stm32l4xx_it.c
    azure_config.h
    board_init.c
    console.c
    main.c
    stm_networking.c
    sample_pnp_iot_device.c
    sample_pnp_device_component.c
    sample_nx_verified_telemetry_init.c
    sample_vt_device_driver.c
)

function(create_target TARGET STARTUP LINKER)
    add_executable(${TARGET}
        ${SOURCES}
        ${STARTUP}
    )

    target_link_libraries(${TARGET}
        azrtos::threadx
        azrtos::netxduo
	    az::iot::vt

        stm32cubel4
        netx_driver
        app_common
        jsmn
    )

    target_link_options(${TARGET}
        PUBLIC 
            -T${LINKER} -Wl,-Map=${TARGET}.map
    )

    set_target_properties(${TARGET}
        PROPERTIES 
            LINK_DEPENDS ${LINKER}
            SUFFIX ".elf"
    )

    target_include_directories(${TARGET} 
        PUBLIC 
            .
    )

    create_bin_output(${TARGET})
    firmware_size(${TARGET})
endfunction()

create_target("stm32l475_azure_iot" "startup/startup_stm32l475xx.s" "${CMAKE_CURRENT_SOURCE_DIR}/startup/STM32L475VGTx_FLASH.ld")
create_target("stm32l4S5_azure_iot" "startup/startup_stm32l4s5xx.s" "${CMAKE_CURRENT_SOURCE_DIR}/startup/STM32L4S5VITx_FLASH.ld")
