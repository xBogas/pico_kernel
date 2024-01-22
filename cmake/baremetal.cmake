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


	message("Including rp2_common")
	set(DIR_RP2_COMMON "lib/pico-sdk/src/rp2_common") 
	pico_add_subdirectory(${DIR_RP2_COMMON}/boot_stage2)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_unique_id)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_bit_ops)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_divider)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_double)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_int64_ops)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_flash)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_float)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_mem_ops)

	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_rand)
	pico_add_subdirectory(${DIR_RP2_COMMON}/cmsis)

	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_cxx_options)
	pico_add_subdirectory(${DIR_RP2_COMMON}/pico_standard_link)

	# link libraries with wrappers and need to be included in the project
	target_link_libraries(${PROJECT_NAME} 
							pico_bit_ops
							pico_divider
							pico_time
							pico_double
							pico_int64_ops
							pico_int64_ops
							pico_float
							pico_float
							cmsis_core
							)
endif()