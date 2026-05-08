add_library(smtc_lbm)

set(SWDR007_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SWDR007_v1-2-0")
set(SWDR007_LBM "${SWDR007_DIR}/swdr007_silabs_lbm_v450/smtc_modem_core")
set(SWDR007_SIDEWALK "${SWDR007_DIR}/swdr007_silabs_lbm_v4_sidewalk")

# global includes
target_include_directories(smtc_lbm
PUBLIC
    ${SWDR007_LBM}/smtc_ral/src
    ${SWDR007_LBM}/smtc_ralf/src
    ${SWDR007_LBM}/geolocation_services
    ${SWDR007_LBM}
    ${SWDR007_LBM}/../smtc_modem_api
    ${SWDR007_LBM}/../smtc_modem_hal
    ${SWDR007_SIDEWALK}/RALBSP
)


# hardcoded configurations
target_compile_definitions(smtc_lbm PUBLIC RP2_103 NUMBER_OF_STACKS=1 LR11XX REGION_US_915)
set(CONFIG_REGION_US_915 ON)

if(DEFINED PORTING_TEST)
    set(LBM_SOURCE
        ${RADIO_SRC}
    )
else()
    set(LBM_SOURCE
        ${SWDR007_LBM}/smtc_modem.c
        ${SWDR007_LBM}/modem_utilities/modem_core.c
        ${SWDR007_LBM}/modem_utilities/modem_event_utilities.c
        ${SWDR007_LBM}/modem_utilities/fifo_ctrl.c
        ${SWDR007_LBM}/modem_supervisor/modem_supervisor_light.c
        ${SWDR007_LBM}/modem_supervisor/modem_tx_protocol_manager.c
        ${SWDR007_LBM}/lorawan_api/lorawan_api.c
        ${SWDR007_LBM}/radio_planner/src/radio_planner.c
        ${SWDR007_LBM}/smtc_modem_crypto/soft_secure_element/soft_se.c # or smtc_modem_crypto/lr11xx_crypto_engine/lr11xx_ce.c
        ${SWDR007_LBM}/lr1mac/src/smtc_real/src/smtc_real.c
        ${SWDR007_LBM}/lr1mac/src/services/smtc_duty_cycle.c
        ${SWDR007_LBM}/lr1mac/src/lr1mac_core.c
        ${SWDR007_LBM}/lr1mac/src/lr1_stack_mac_layer.c
        ${SWDR007_LBM}/lr1mac/src/lr1mac_utilities.c
        ${SWDR007_LBM}/lr1mac/src/services/smtc_lbt.c
        ${SWDR007_LBM}/lorawan_manager/lorawan_send_management.c
        ${SWDR007_LBM}/lorawan_manager/lorawan_join_management.c
        ${SWDR007_LBM}/lorawan_manager/lorawan_dwn_ack_management.c
        ${SWDR007_LBM}/lorawan_manager/lorawan_cid_request_management.c
        ${SWDR007_LBM}/lorawan_packages/lorawan_certification/lorawan_certification.c
        ${SWDR007_LBM}/smtc_modem_crypto/smtc_modem_crypto.c
        ${SWDR007_LBM}/smtc_modem_crypto/soft_secure_element/cmac.c
        ${SWDR007_LBM}/smtc_modem_crypto/soft_secure_element/aes.c
        ${SWDR007_LBM}/smtc_modem_test.c
        ${SWDR007_LBM}/modem_services/cloud_dm_package/cloud_dm_package.c
        ${SWDR007_LBM}/modem_services/lfu_service/file_upload.c
        ${SWDR007_LBM}/lr1mac/src/services/smtc_lora_cad_bt.c
        ${SWDR007_LBM}/lorawan_packages/application_layer_clock_synchronization/v2.0.0/lorawan_alcsync_v2.0.0.c
        ${SWDR007_SIDEWALK}/RALBSP/ral_lr11xx_bsp.c
    )

    target_compile_definitions(smtc_lbm PUBLIC ADD_LBM_GEOLOCATION)
    set(LBM_SOURCE ${LBM_SOURCE}
        ${SWDR007_LBM}/geolocation_services/mw_gnss_almanac.c
        ${SWDR007_LBM}/geolocation_services/mw_gnss_scan.c
        ${SWDR007_LBM}/geolocation_services/mw_wifi_scan.c
        ${SWDR007_LBM}/geolocation_services/mw_wifi_send.c
        ${SWDR007_LBM}/geolocation_services/mw_common.c
        ${SWDR007_LBM}/geolocation_services/wifi_helpers.c
        ${SWDR007_LBM}/geolocation_services/gnss_helpers.c
        ${SWDR007_LBM}/geolocation_services/mw_gnss_send.c
    )
endif()

if(SID_RADIO_PLATFORM STREQUAL lr11xx OR SID_RADIO_PLATFORM STREQUAL lr11xx_exp)
    set(LBM_SOURCE ${LBM_SOURCE}
        ${SWDR007_LBM}/smtc_ralf/src/ralf_lr11xx.c
        ${SWDR007_LBM}/smtc_ral/src/ral_lr11xx.c
    )
else()
    set(LBM_SOURCE ${LBM_SOURCE}
        ${SWDR007_LBM}/smtc_ral/src/ral_sx126x.c
        ${SWDR007_LBM}/smtc_ralf/src/ralf_sx126x.c
        ${SWDR007_LBM}/radio_drivers/sx126x_driver/src/sx126x_lr_fhss.c
        ${SWDR007_LBM}/radio_drivers/sx126x_driver/src/lr_fhss_mac.c
    )
endif()

target_compile_definitions(smtc_lbm INTERFACE
    NUMBER_OF_STACKS=1 # NB_OF_STACK
)

if (CONFIG_TEST_BYPASS_JOIN_DUTY_CYCLE)
    target_compile_definitions(smtc_lbm INTERFACE TEST_BYPASS_JOIN_DUTY_CYCLE)
endif()

if(SID_RADIO_PLATFORM STREQUAL lr11xx OR SID_RADIO_PLATFORM STREQUAL lr11xx_exp)
    target_compile_definitions(smtc_lbm INTERFACE LR11XX)
else()
    message(SEND_ERROR "board definition for other radio SX128X, SX126X, SX127X")
endif()

target_compile_definitions(smtc_lbm PUBLIC
    SMTC_MODEM_HAL_IRQ_FROM_SID_PAL
    WIFI_SCAN_DEEP_DBG_TRACE
    GNSS_ALMANAC_DEEP_DBG_TRACE
    GNSS_SCAN_DEEP_DBG_TRACE
)

# includes private to LBM
if(DEFINED PORTING_TEST)
    target_compile_definitions(smtc_lbm INTERFACE PORTING_TEST)
    target_include_directories(smtc_lbm INTERFACE
        ${SWDR007_LBM}/radio_drivers/sx126x_driver/src
    )
else()
    if (CONFIG_REGION_US_915)
        target_compile_definitions(smtc_lbm INTERFACE REGION_US_915)
        set(LBM_SOURCE ${LBM_SOURCE} ${SWDR007_LBM}/lr1mac/src/smtc_real/src/region_us_915.c)
    elseif (CONFIG_REGION_EU868)
        target_compile_definitions(smtc_lbm INTERFACE REGION_EU_868)
        set(LBM_SOURCE ${LBM_SOURCE} ${SWDR007_LBM}/lr1mac/src/smtc_real/src/region_eu_868.c)
    else()
        message(FATAL_ERROR "other region defined")
    endif()

    target_include_directories(smtc_lbm PUBLIC
        ${SWDR007_LBM}/lorawan_manager
        ${SWDR007_LBM}/lr1mac/src
        ${SWDR007_LBM}/smtc_modem_crypto/smtc_secure_element
        ${SWDR007_LBM}/lr1mac/src/smtc_real/src
        ${SWDR007_LBM}/radio_planner/src
        ${SWDR007_LBM}/lr1mac/src/services
        ${SWDR007_LBM}/modem_supervisor
        ${SWDR007_LBM}/modem_utilities
        ${SWDR007_LBM}/lorawan_packages/lorawan_certification
        ${SWDR007_LBM}/lorawan_api
        ${SWDR007_LBM}/smtc_modem_crypto
        ${SWDR007_LBM}/..
        ${SWDR007_LBM}/modem_config
        ${SWDR007_LBM}/lr1mac/src/services/smtc_multicast
        ${SWDR007_LBM}/lr1mac
        ${SWDR007_LBM}/modem_services/cloud_dm_package
        ${SWDR007_LBM}/modem_services
        ${SWDR007_LBM}/modem_services/lfu_service
        ${SWDR007_LBM}/lorawan_packages/application_layer_clock_synchronization
    )
endif()

target_sources(smtc_lbm PUBLIC ${LBM_SOURCE})
target_link_libraries(smtc_lbm
PUBLIC
    sid_pal_timer_ifc
    sid_clock_ifc
    sid_pal_gpio_ifc
    sid_pal_mfg_store_ifc
    sid_900_cfg_ifc
    sid_pal_log_ifc
PRIVATE
    sid_pal_radio_lr11xx_exp_impl
)