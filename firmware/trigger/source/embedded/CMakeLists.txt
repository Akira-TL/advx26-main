##
# @file CMakeLists.txt
# @brief Embedded application top-level CMake — mirrors the layout
#        produced by `tos.py new project`. Lives under
#        `<project>/source/embedded/`; build with
#        `cd source/embedded && tos.py build`.
#/

# APP_PATH
set(APP_PATH ${CMAKE_CURRENT_LIST_DIR})

# APP_NAME
get_filename_component(APP_NAME ${APP_PATH} NAME)

# APP_SRC
aux_source_directory(${APP_PATH}/src APP_SRC)

# USR_BOARD_SRC
aux_source_directory(${APP_PATH}/usr_board USR_BOARD_SRC)

# APP_INC
set(APP_INC ${APP_PATH}/include ${APP_PATH}/usr_board)

# APP_OPTIONS
set(APP_OPTIONS "-W")
list(APPEND APP_OPTIONS "-Wall" "-DLV_LVGL_H_INCLUDE_SIMPLE")

########################################
# Target Configure
########################################
add_library(${EXAMPLE_LIB})
message(STATUS "EXAMPLE_LIB:${APP_PATH}")

target_sources(${EXAMPLE_LIB}
    PRIVATE
        ${APP_SRC}
        ${USR_BOARD_SRC}
    )

target_include_directories(${EXAMPLE_LIB}
    PRIVATE
        ${APP_INC}
    )

target_compile_options(${EXAMPLE_LIB}
    PRIVATE
        ${APP_OPTIONS}
    )
