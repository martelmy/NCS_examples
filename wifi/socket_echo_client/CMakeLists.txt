# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sockets_echo_client)

if(CONFIG_MBEDTLS_KEY_EXCHANGE_PSK_ENABLED AND
    (CONFIG_NET_SAMPLE_PSK_HEADER_FILE STREQUAL "dummy_psk.h"))
  add_custom_target(development_psk
    COMMAND ${CMAKE_COMMAND} -E echo "----------------------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo "--- WARNING: Using dummy PSK! Only suitable for        ---"
    COMMAND ${CMAKE_COMMAND} -E echo "--- development. Set NET_SAMPLE_PSK_HEADER_FILE to use ---"
    COMMAND ${CMAKE_COMMAND} -E echo "--- own pre-shared key.                                ---"
    COMMAND ${CMAKE_COMMAND} -E echo "----------------------------------------------------------"
  )
  add_dependencies(app development_psk)
endif()

zephyr_library_include_directories(src)

target_sources(app PRIVATE
    src/main.c
    src/network.c)

set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)

generate_inc_file_for_target(
    app
    src/echo-apps-cert.der
    ${gen_dir}/echo-apps-cert.der.inc
    )
