cmake_minimum_required(VERSION 3.15)
set(CMAKE_C_STANDARD 11)

set(PICO_BARE_METAL 1)
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

set(TRUE_BAREMETAL FALSE)
include(cmake/baremetal.cmake)

add_compile_options(-Wall
                    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
                    -Wno-unused-function # we have some for the docs that aren't called
                    )

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

add_compile_definitions(PICO_HEAP_SIZE=0)
add_compile_definitions(PICO_STACK_SIZE=0x1000)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${SRC_ASM_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# pull in common dependencies
target_link_libraries(${PROJECT_NAME}
                        pico_standard_link
                        hardware_uart
                        cmsis_core
                        )

target_link_options(${PROJECT_NAME} PUBLIC "--specs=nosys.specs")
pico_wrap_function(${PROJECT_NAME} malloc)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})