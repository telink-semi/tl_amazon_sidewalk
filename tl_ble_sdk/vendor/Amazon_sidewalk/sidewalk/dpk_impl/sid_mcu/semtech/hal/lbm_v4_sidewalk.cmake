#
# Copyright 2024 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.txt file. This file is a
# Modifiable File, as defined in the accompanying LICENSE.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
#


add_library(lbm_sidewalk)

set(SWDR007_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SWDR007_v1-2-0")
set(SWDR007_LBM_v450 "${SWDR007_DIR}/swdr007_silabs_lbm_v450")
set(SWDR007_LBM_v4_SIDEWALK "${SWDR007_DIR}/swdr007_silabs_lbm_v4_sidewalk")

target_include_directories(lbm_sidewalk
PUBLIC
    ${SWDR007_LBM_v4_SIDEWALK}/RALBSP
)

if(SID_RADIO_PLATFORM STREQUAL lr11xx)
    set(RADIO_SRC
        ${SWDR007_LBM_v4_SIDEWALK}/RALBSP/ral_lr11xx_bsp.c
    )
endif()

set(SRC
    ${SWDR007_LBM_v4_SIDEWALK}/ModemHAL/smtc_modem_hal.c
    ${RADIO_SRC}
)



target_include_directories(lbm_sidewalk INTERFACE
    ${SWDR007_LBM_v450}/smtc_modem_core
)

if(SID_RADIO_PLATFORM STREQUAL lr11xx)
else()
    target_include_directories(lbm_sidewalk INTERFACE
        ../${LBM_DIR}/smtc_modem_core/radio_drivers/sx126x_driver/src
        sx126x/include
        sx126x/include/semtech
    )
endif()

if(DEFINED PORTING_TEST) # LBM lib not built
    target_include_directories(lbm_sidewalk INTERFACE
        ${SWDR007_LBM_v450}/smtc_modem_core/smtc_ral/src
        ${SWDR007_LBM_v450}/smtc_modem_core/smtc_ralf/src
        ${SWDR007_LBM_v450}/smtc_modem_hal
        ${SWDR007_LBM_v450}/smtc_modem_api
    )
    target_compile_definitions(lbm_sidewalk INTERFACE PORTING_TEST)
endif()

if(SID_RADIO_PLATFORM STREQUAL lr11xx)
    target_compile_definitions(lbm_sidewalk INTERFACE LR11XX)
else()
    message(SEND_ERROR "board definition for other radio SX128X, SX126X, SX127X")
endif()

if(CONFIG_LBM_REBOOT_ON_PANIC)
    target_compile_definitions(lbm_sidewalk INTERFACE LBM_REBOOT_ON_PANIC)
endif()

target_sources(lbm_sidewalk PUBLIC ${SRC})

target_link_libraries(lbm_sidewalk
PUBLIC
    sid_pal_radio_lr11xx_impl
    sid_pal_timer_ifc
    sid_900_cfg_ifc
    sid_pal_mfg_store_ifc
    sid_clock_ifc
    sid_pal_gpio_ifc
    smtc_lbm
)