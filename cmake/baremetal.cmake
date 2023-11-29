if(PICO_BARE_METAL)
	message("Manualling including non-baremetal libraries")

	message("Including common")
	set(DIR_COMMON "lib/pico-sdk/src/common") 
	pico_add_subdirectory(${DIR_COMMON}/pico_bit_ops)
	add_library(pico_binary_info INTERFACE)
	pico_add_subdirectory(${DIR_COMMON}/pico_binary_info)
	pico_add_subdirectory(${DIR_COMMON}/pico_divider)
	pico_add_subdirectory(${DIR_COMMON}/pico_sync)
	pico_add_subdirectory(${DIR_COMMON}/pico_time)
	pico_add_subdirectory(${DIR_COMMON}/pico_util)

	pico_add_subdirectory(${DIR_COMMON}/pico_stdlib)

	message("Including rp2_common")
	set(DIR_RP2_COMMON "lib/pico-sdk/src/rp2_common") 
	pico_add_subdirectory(${DIR_RP2_COMMON}/boot_stage2)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_bootsel_via_double_reset)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_multicore)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_unique_id)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_bit_ops)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_divider)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_double)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_int64_ops)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_flash)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_float)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_mem_ops)
	# pico_add_subdirectory(${DIR_RP2_COMMON}/pico_malloc)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_printf)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_rand)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_stdio)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_stdio_semihosting)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_stdio_uart)
	pico_add_subdirectory(${DIR_RP2_COMMON}/cmsis)
	pico_add_subdirectory(${DIR_RP2_COMMON}/hardware_exception)
	set(PICO_TINYUSB_PATH ${CMAKE_SOURCE_DIR}/lib/pico-sdk/lib/tinyusb)
	pico_add_subdirectory(${DIR_RP2_COMMON}/tinyusb)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_stdio_usb)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_i2c_slave)

	# networking libraries - note dependency order is important
	# pico_add_subdirectory(${DIR_RP2_COMMON}/pico_async_context)
	# pico_add_subdirectory(${DIR_RP2_COMMON}/pico_btstack)
	# pico_add_subdirectory(${DIR_RP2_COMMON}/pico_cyw43_driver)
	# pico_add_subdirectory(${DIR_RP2_COMMON}/pico_lwip)
	# pico_add_subdirectory(${DIR_RP2_COMMON}/pico_cyw43_arch)
	# pico_add_subdirectory(${DIR_RP2_COMMON}/pico_mbedtls)

	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_stdlib)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_cxx_options)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_standard_link)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_fix)
	#pico_add_subdirectory(${DIR_RP2_COMMON}/pico_runtime)
	
	# substitute for pico_runtime
	pico_add_library(pico_runtime)
	target_sources(pico_runtime INTERFACE ${CMAKE_SOURCE_DIR}/src/init.c)

	target_include_directories(pico_runtime_headers INTERFACE ${CMAKE_SOURCE_DIR}/include)

	pico_mirrored_target_link_libraries(pico_runtime INTERFACE
			hardware_uart
			hardware_clocks
			hardware_irq
			pico_printf
			pico_sync
			)

	if (TARGET pico_bit_ops)
		pico_mirrored_target_link_libraries(pico_runtime INTERFACE pico_bit_ops)
	endif()
	if (TARGET pico_divider)
		pico_mirrored_target_link_libraries(pico_runtime INTERFACE pico_divider)
	endif()
	if (TARGET pico_double)
		pico_mirrored_target_link_libraries(pico_runtime INTERFACE pico_double)
	endif()
	if (TARGET pico_int64_ops)
		pico_mirrored_target_link_libraries(pico_runtime INTERFACE pico_int64_ops)
	endif()
	if (TARGET pico_float)
		pico_mirrored_target_link_libraries(pico_runtime INTERFACE pico_float)
	endif()
	if (TARGET pico_malloc)
		pico_mirrored_target_link_libraries(pico_runtime INTERFACE pico_malloc)
	endif()
	if (TARGET pico_mem_ops)
		pico_mirrored_target_link_libraries(pico_runtime INTERFACE pico_mem_ops)
	endif()
	if (TARGET pico_standard_link)
		pico_mirrored_target_link_libraries(pico_runtime INTERFACE pico_standard_link)
	endif()

	target_link_options(pico_runtime INTERFACE "--specs=nosys.specs" )
endif()