message(STATUS "Adding configuration for WIFI module - ${CMAKE_CURRENT_LIST_DIR}/include")

set(WIFI_CONFIG ${CMAKE_CURRENT_LIST_DIR}/include/wifi_config.h)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)


# networking libraries - note dependency order is important
pico_add_subdirectory(${DIR_RP2_COMMON}/pico_async_context)
# pico_add_subdirectory(${DIR_RP2_COMMON}/pico_btstack)
pico_add_subdirectory(${DIR_RP2_COMMON}/pico_cyw43_driver)
pico_add_subdirectory(${DIR_RP2_COMMON}/pico_lwip)
pico_add_subdirectory(${DIR_RP2_COMMON}/pico_cyw43_arch)
# pico_add_subdirectory(${DIR_RP2_COMMON}/pico_mbedtls)
