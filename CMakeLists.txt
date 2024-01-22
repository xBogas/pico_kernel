cmake_minimum_required(VERSION 3.15)
set(CMAKE_C_STANDARD 11)

set(PICO_BARE_METAL 1) # dont include all the pico stuff - only hardware structs
set(CMAKE_BUILD_TYPE Debug)
set(PICO_DEOPTIMIZED_DEBUG 1)

set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/pico-sdk)
set(PICO_BOARD pico_w)
set(PICO_PLATFORM rp2040)

# Pull in SDK (must be before project)

include(cmake/pico_sdk_import.cmake)
include(cmake/pico_extras_import.cmake)


project(firmware C CXX ASM)

include_directories(${PROJECT_NAME} PUBLIC include)
file(GLOB_RECURSE SRC_FILES src/*.c)
file(GLOB_RECURSE SRC_ASM_FILES src/*.S)

# Initialize the SDK
pico_sdk_init()

add_executable(${PROJECT_NAME} ${SRC_FILES} ${SRC_ASM_FILES})

include(cmake/baremetal.cmake)
include(config/wifi_config.cmake)

add_compile_options(-Wall
                    -Wno-maybe-uninitialized
                    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
                    -Wno-unused-function # we have some for the docs that aren't called
                    )

# set no heap - dont allow linker script to allocate heap
add_compile_definitions(PICO_HEAP_SIZE=0)
add_compile_definitions(PICO_STACK_SIZE=0x1000) # Stack size for each core


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# pull in common dependencies
target_link_libraries(${PROJECT_NAME}
                        pico_standard_link
                        hardware_uart
                        # pico_cyw43_arch_lwip_threadsafe_background
                        )

# if(WIFI_CONFIG) background thread need to be priveleged
#     target_compile_definitions(${PROJECT_NAME} PUBLIC 
#         CYW43_CUSTOM_CONFIG="${WIFI_CONFIG}"
#         USE_PRIV_THREADS)
# endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC 
    USE_PRIV_THREADS)

target_link_options(${PROJECT_NAME} PUBLIC "--specs=nosys.specs")

# wrapper for malloc
# newlibc malloc implementation becomes __real_malloc
# malloc calls are the redirected to __wrap_malloc that needs to be implemented
target_link_options(${PROJECT_NAME} PUBLIC "LINKER:--wrap=malloc")

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})

# create symbol table
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND sh -c "${CMAKE_OBJDUMP} -t ${PROJECT_NAME}.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > ${PROJECT_NAME}.sym"
    VERBATIM
)