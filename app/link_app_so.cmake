# Automatically collect all ESP-IDF component .a files and link them into app.so

# Collect all .a files from the ESP-IDF build directory
file(GLOB_RECURSE ALL_ARCHIVES "${ESP_IDF_LIB_DIR}/*.a")

# Filter
set(LINK_ARCHIVES "")
foreach(archive ${ALL_ARCHIVES})
    if(archive MATCHES "CMakeFiles")
        continue()
    endif()
    if(archive MATCHES "cmock|unity")
        continue()
    endif()
    # Skip low-level system libraries with assembly incompatible with -shared
    if(archive MATCHES "libriscv\\.a|libesp_system\\.a|libesp_rom\\.a|libbootloader_support\\.a|libesp_app_format\\.a|libesp_bootloader_format\\.a|libapp_update\\.a|libesp_gdbstub\\.a|libespcoredump\\.a|libnvs_sec_provider\\.a|libesp_security\\.a|librt\\.a|libstdc\\+\\+\\.a|libcxx\\.a|libbsp_stub\\.a")
        continue()
    endif()
    list(APPEND LINK_ARCHIVES ${archive})
endforeach()

list(SORT LINK_ARCHIVES)

# Separate main library
set(MAIN_ARCHIVE "")
set(OTHER_ARCHIVES "")
foreach(archive ${LINK_ARCHIVES})
    if(archive MATCHES "/main/libmain\\.a$")
        set(MAIN_ARCHIVE ${archive})
    else()
        list(APPEND OTHER_ARCHIVES ${archive})
    endif()
endforeach()

if(NOT MAIN_ARCHIVE)
    message(FATAL_ERROR "Could not find libmain.a in ${ESP_IDF_LIB_DIR}")
endif()

list(LENGTH OTHER_ARCHIVES num_libs)
message(STATUS "Linking app.so with libmain.a + ${num_libs} component libraries")

# Collect fakelibs
file(GLOB FAKELIBS "${FAKELIB_DIR}/*.so")
set(FAKELIB_FLAGS "")
foreach(fakelib ${FAKELIBS})
    get_filename_component(libname ${fakelib} NAME_WE)
    string(REGEX REPLACE "^lib" "" libname ${libname})
    list(APPEND FAKELIB_FLAGS "-l${libname}")
endforeach()

# Link
execute_process(
    COMMAND ${CC}
        -shared
        -nostdlib
        -Wl,--gc-sections
        -Wl,-Bsymbolic
        -Wl,-T,${LD_SCRIPT}
        -Wl,-Map=${MAP_FILE}
        -Wl,--whole-archive
        ${MAIN_ARCHIVE}
        -Wl,--no-whole-archive
        -Wl,--start-group
        ${OTHER_ARCHIVES}
        -Wl,--end-group
        -L${FAKELIB_DIR}
        ${FAKELIB_FLAGS}
        -lgcc
        -o ${OUTPUT}
    RESULT_VARIABLE link_result
    OUTPUT_VARIABLE link_output
    ERROR_VARIABLE link_error
)

if(NOT link_result EQUAL 0)
    message(FATAL_ERROR "Linking app.so failed:\n${link_error}")
endif()

execute_process(
    COMMAND ${SIZE} ${OUTPUT}
    OUTPUT_VARIABLE size_output
)
message(STATUS "app.so size:\n${size_output}")
