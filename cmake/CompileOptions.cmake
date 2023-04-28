# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

set(QPL_LINUX_TOOLCHAIN_REQUIRED_FLAGS "-Wall;-Wextra;-fPIC;-fstack-protector;-fstack-protector-strong;-fstack-clash-protection;--param=ssp-buffer-size=4")
set(QPL_LINUX_TOOLCHAIN_DYNAMIC_LIBRARY_FLAGS "-Wl,-z,relro,-z,now;-Wl,-z,noexecstack")
set(QPL_LINUX_TOOLCHAIN_CPP_EMBEDDED_FLAGS "-fno-exceptions;-fno-rtti")

set(QPL_WINDOWS_TOOLCHAIN_REQUIRED_FLAGS "/W3;/WX;/GS")
set(QPL_WINDOWS_TOOLCHAIN_DYNAMIC_LIBRARY_FLAGS "")
set(QPL_WINDOWS_TOOLCHAIN_CPP_EMBEDDED_FLAGS "/EHsc")

function(modify_standard_language_flag)
    # Declaring function parameters
    set(OPTIONS "")
    set(ONE_VALUE_ARGS
        LANGUAGE_NAME
        FLAG_NAME
        NEW_FLAG_VALUE)
    set(MULTI_VALUE_ARGS "")

    # Parsing function parameters
    cmake_parse_arguments(MODIFY
                          "${OPTIONS}"
                          "${ONE_VALUE_ARGS}"
                          "${MULTI_VALUE_ARGS}"
                          ${ARGN})

    # Variables
    set(FLAG_REGULAR_EXPRESSION "${MODIFY_FLAG_NAME}.*[ ]*")
    set(NEW_VALUE "${MODIFY_FLAG_NAME}${MODIFY_NEW_FLAG_VALUE}")

    # Replacing specified flag with new value
    string(REGEX REPLACE
           ${FLAG_REGULAR_EXPRESSION} ${NEW_VALUE}
           NEW_COMPILE_FLAGS
           "${CMAKE_${MODIFY_LANGUAGE_NAME}_FLAGS}")

    # Returning the value
    set(CMAKE_${MODIFY_LANGUAGE_NAME}_FLAGS ${NEW_COMPILE_FLAGS} PARENT_SCOPE)
endfunction()

function(get_function_name_with_default_bit_width in_function_name bit_width out_function_name)

    if(in_function_name MATCHES ".*_i")

        string(REPLACE "_i" "" in_function_name ${in_function_name})

        set(${out_function_name} "${in_function_name}_${bit_width}_i" PARENT_SCOPE)

    else()

        set(${out_function_name} "${in_function_name}_${bit_width}" PARENT_SCOPE)

    endif()

endfunction()

macro(get_list_of_supported_optimizations PLATFORMS_LIST)
    list(APPEND PLATFORMS_LIST "")
    list(APPEND PLATFORMS_LIST "px")
    list(APPEND PLATFORMS_LIST "avx512")
endmacro(get_list_of_supported_optimizations)

function(generate_unpack_kernel_arrays current_directory PLATFORMS_LIST)
    list(APPEND UNPACK_POSTFIX_LIST "")
    list(APPEND UNPACK_PRLE_POSTFIX_LIST "")
    list(APPEND PACK_POSTFIX_LIST "")
    list(APPEND PACK_INDEX_POSTFIX_LIST "")
    list(APPEND SCAN_POSTFIX_LIST "")
    list(APPEND DEFAULT_BIT_WIDTH_FUNCTIONS_LIST "")
    list(APPEND DEFAULT_BIT_WIDTH_LIST "")

    #create list of functions that use only 8u 16u 32u postfixes
    list(APPEND DEFAULT_BIT_WIDTH_FUNCTIONS_LIST "unpack_prle")
    list(APPEND DEFAULT_BIT_WIDTH_FUNCTIONS_LIST "extract")
    list(APPEND DEFAULT_BIT_WIDTH_FUNCTIONS_LIST "extract_i")
    list(APPEND DEFAULT_BIT_WIDTH_FUNCTIONS_LIST "select")
    list(APPEND DEFAULT_BIT_WIDTH_FUNCTIONS_LIST "select_i")
    list(APPEND DEFAULT_BIT_WIDTH_FUNCTIONS_LIST "expand")

    #create default bit width list
    list(APPEND DEFAULT_BIT_WIDTH_LIST "8u")
    list(APPEND DEFAULT_BIT_WIDTH_LIST "16u")
    list(APPEND DEFAULT_BIT_WIDTH_LIST "32u")

    #create scan kernel postfixes
    list(APPEND SCAN_COMPARATOR_LIST "")

    list(APPEND SCAN_COMPARATOR_LIST "eq")
    list(APPEND SCAN_COMPARATOR_LIST "ne")
    list(APPEND SCAN_COMPARATOR_LIST "lt")
    list(APPEND SCAN_COMPARATOR_LIST "le")
    list(APPEND SCAN_COMPARATOR_LIST "gt")
    list(APPEND SCAN_COMPARATOR_LIST "ge")
    list(APPEND SCAN_COMPARATOR_LIST "range")
    list(APPEND SCAN_COMPARATOR_LIST "not_range")

    foreach(SCAN_COMPARATOR IN LISTS SCAN_COMPARATOR_LIST)
        list(APPEND SCAN_POSTFIX_LIST "_${SCAN_COMPARATOR}_8u")
        list(APPEND SCAN_POSTFIX_LIST "_${SCAN_COMPARATOR}_16u8u")
        list(APPEND SCAN_POSTFIX_LIST "_${SCAN_COMPARATOR}_32u8u")
    endforeach()

    # create unpack kernel postfixes
    foreach(input_width RANGE 1 32 1)
        if(input_width LESS 8 OR input_width EQUAL 8)
            list(APPEND UNPACK_POSTFIX_LIST "_${input_width}u8u")

        elseif(input_width LESS 16 OR input_width EQUAL 16)
            list(APPEND UNPACK_POSTFIX_LIST "_${input_width}u16u")

        else()
            list(APPEND UNPACK_POSTFIX_LIST "_${input_width}u32u")
        endif()
    endforeach()

    # create pack kernel postfixes
    foreach(output_width RANGE 1 8 1)
        list(APPEND PACK_POSTFIX_LIST "_8u${output_width}u")
    endforeach()

    foreach(output_width RANGE 9 16 1)
        list(APPEND PACK_POSTFIX_LIST "_16u${output_width}u")
    endforeach()

    foreach(output_width RANGE 17 32 1)
        list(APPEND PACK_POSTFIX_LIST "_32u${output_width}u")
    endforeach()

    list(APPEND PACK_POSTFIX_LIST "_8u16u")
    list(APPEND PACK_POSTFIX_LIST "_8u32u")
    list(APPEND PACK_POSTFIX_LIST "_16u32u")

    # create pack index kernel postfixes
    list(APPEND PACK_INDEX_POSTFIX_LIST "_nu")
    list(APPEND PACK_INDEX_POSTFIX_LIST "_8u")
    list(APPEND PACK_INDEX_POSTFIX_LIST "_8u16u")
    list(APPEND PACK_INDEX_POSTFIX_LIST "_8u32u")

    # write to file
    file(MAKE_DIRECTORY ${current_directory}/generated)

    foreach(PLATFORM_VALUE IN LISTS PLATFORMS_LIST)
        set(directory "${current_directory}/generated")
        set(PLATFORM_PREFIX "${PLATFORM_VALUE}_")

        #
        # Write unpack table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}unpack.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}unpack.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}unpack.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}unpack.cpp "unpack_table_t ${PLATFORM_PREFIX}unpack_table = {\n")

        #write LE kernels
        foreach(UNPACK_POSTFIX IN LISTS UNPACK_POSTFIX_LIST)
            file(APPEND ${directory}/${PLATFORM_PREFIX}unpack.cpp "\t${PLATFORM_PREFIX}qplc_unpack${UNPACK_POSTFIX},\n")
        endforeach()

        #write BE kernels

        #get last element of the list
        set(LAST_ELEMENT "")
        list(GET UNPACK_POSTFIX_LIST -1 LAST_ELEMENT)

        foreach(UNPACK_POSTFIX IN LISTS UNPACK_POSTFIX_LIST)

            if(UNPACK_POSTFIX STREQUAL LAST_ELEMENT)
                file(APPEND ${directory}/${PLATFORM_PREFIX}unpack.cpp "\t${PLATFORM_PREFIX}qplc_unpack_be${UNPACK_POSTFIX}};\n")
            else()
                file(APPEND ${directory}/${PLATFORM_PREFIX}unpack.cpp "\t${PLATFORM_PREFIX}qplc_unpack_be${UNPACK_POSTFIX},\n")
            endif()
        endforeach()

        file(APPEND ${directory}/${PLATFORM_PREFIX}unpack.cpp "}\n")

        #
        # Write pack table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}pack.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack.cpp "pack_table_t ${PLATFORM_PREFIX}pack_table = {\n")

        #write LE kernels
        foreach(PACK_POSTFIX IN LISTS PACK_POSTFIX_LIST)
            file(APPEND ${directory}/${PLATFORM_PREFIX}pack.cpp "\t${PLATFORM_PREFIX}qplc_pack${PACK_POSTFIX},\n")
        endforeach()

        #write BE kernels

        #get last element of the list
        set(LAST_ELEMENT "")
        list(GET PACK_POSTFIX_LIST -1 LAST_ELEMENT)

        foreach(PACK_POSTFIX IN LISTS PACK_POSTFIX_LIST)

            if(PACK_POSTFIX STREQUAL LAST_ELEMENT)
                file(APPEND ${directory}/${PLATFORM_PREFIX}pack.cpp "\t${PLATFORM_PREFIX}qplc_pack_be${PACK_POSTFIX}};\n")
            else()
                file(APPEND ${directory}/${PLATFORM_PREFIX}pack.cpp "\t${PLATFORM_PREFIX}qplc_pack_be${PACK_POSTFIX},\n")
            endif()
        endforeach()

        file(APPEND ${directory}/${PLATFORM_PREFIX}pack.cpp "}\n")

        #
        # Write scan table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}scan.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}scan.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}scan.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}scan.cpp "scan_table_t ${PLATFORM_PREFIX}scan_table = {\n")

        #get last element of the list
        set(LAST_ELEMENT "")
        list(GET SCAN_POSTFIX_LIST -1 LAST_ELEMENT)

        foreach(SCAN_POSTFIX IN LISTS SCAN_POSTFIX_LIST)

            if(SCAN_POSTFIX STREQUAL LAST_ELEMENT)
                file(APPEND ${directory}/${PLATFORM_PREFIX}scan.cpp "\t${PLATFORM_PREFIX}qplc_scan${SCAN_POSTFIX}};\n")
            else()
                file(APPEND ${directory}/${PLATFORM_PREFIX}scan.cpp "\t${PLATFORM_PREFIX}qplc_scan${SCAN_POSTFIX},\n")
            endif()
        endforeach()

        file(APPEND ${directory}/${PLATFORM_PREFIX}scan.cpp "}\n")

        #
        # Write scan_i table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}scan_i.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}scan_i.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}scan_i.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}scan_i.cpp "scan_i_table_t ${PLATFORM_PREFIX}scan_i_table = {\n")

        #get last element of the list
        set(LAST_ELEMENT "")
        list(GET SCAN_POSTFIX_LIST -1 LAST_ELEMENT)

        foreach(SCAN_POSTFIX IN LISTS SCAN_POSTFIX_LIST)

            if(SCAN_POSTFIX STREQUAL LAST_ELEMENT)
                file(APPEND ${directory}/${PLATFORM_PREFIX}scan_i.cpp "\t${PLATFORM_PREFIX}qplc_scan${SCAN_POSTFIX}_i};\n")
            else()
                file(APPEND ${directory}/${PLATFORM_PREFIX}scan_i.cpp "\t${PLATFORM_PREFIX}qplc_scan${SCAN_POSTFIX}_i,\n")
            endif()
        endforeach()

        file(APPEND ${directory}/${PLATFORM_PREFIX}scan_i.cpp "}\n")

        #
        # Write pack_index table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}pack_index.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "pack_index_table_t ${PLATFORM_PREFIX}pack_index_table = {\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "\t${PLATFORM_PREFIX}qplc_pack_bits_nu,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "\t${PLATFORM_PREFIX}qplc_pack_index_8u,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "\t${PLATFORM_PREFIX}qplc_pack_index_8u16u,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "\t${PLATFORM_PREFIX}qplc_pack_index_8u32u,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "\t${PLATFORM_PREFIX}qplc_pack_bits_be_nu,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "\t${PLATFORM_PREFIX}qplc_pack_index_8u,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "\t${PLATFORM_PREFIX}qplc_pack_index_be_8u16u,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "\t${PLATFORM_PREFIX}qplc_pack_index_be_8u32u};\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}pack_index.cpp "}\n")

        #
        # Write default bit width functions
        #
        foreach(DEAULT_BIT_WIDTH_FUNCTION IN LISTS DEFAULT_BIT_WIDTH_FUNCTIONS_LIST)
            file(WRITE ${directory}/${PLATFORM_PREFIX}${DEAULT_BIT_WIDTH_FUNCTION}.cpp "#include \"qplc_api.h\"\n")
            file(APPEND ${directory}/${PLATFORM_PREFIX}${DEAULT_BIT_WIDTH_FUNCTION}.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
            file(APPEND ${directory}/${PLATFORM_PREFIX}${DEAULT_BIT_WIDTH_FUNCTION}.cpp "namespace qpl::core_sw::dispatcher\n{\n")
            file(APPEND ${directory}/${PLATFORM_PREFIX}${DEAULT_BIT_WIDTH_FUNCTION}.cpp "${DEAULT_BIT_WIDTH_FUNCTION}_table_t ${PLATFORM_PREFIX}${DEAULT_BIT_WIDTH_FUNCTION}_table = {\n")

            #get last element of the list
            set(LAST_ELEMENT "")
            list(GET DEFAULT_BIT_WIDTH_LIST -1 LAST_ELEMENT)

            foreach(BIT_WIDTH IN LISTS DEFAULT_BIT_WIDTH_LIST)

                set(FUNCTION_NAME "")
                get_function_name_with_default_bit_width(${DEAULT_BIT_WIDTH_FUNCTION} ${BIT_WIDTH} FUNCTION_NAME)

                if(BIT_WIDTH STREQUAL LAST_ELEMENT)
                    file(APPEND ${directory}/${PLATFORM_PREFIX}${DEAULT_BIT_WIDTH_FUNCTION}.cpp "\t${PLATFORM_PREFIX}qplc_${FUNCTION_NAME}};\n")
                else()
                 file(APPEND ${directory}/${PLATFORM_PREFIX}${DEAULT_BIT_WIDTH_FUNCTION}.cpp "\t${PLATFORM_PREFIX}qplc_${FUNCTION_NAME},\n")
                 endif()
            endforeach()

            file(APPEND ${directory}/${PLATFORM_PREFIX}${DEAULT_BIT_WIDTH_FUNCTION}.cpp "}\n")
        endforeach()

        #
        # Write aggregates table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}aggregates.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}aggregates.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}aggregates.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}aggregates.cpp "aggregates_table_t ${PLATFORM_PREFIX}aggregates_table = {\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}aggregates.cpp "\t${PLATFORM_PREFIX}qplc_bit_aggregates_8u,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}aggregates.cpp "\t${PLATFORM_PREFIX}qplc_aggregates_8u,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}aggregates.cpp "\t${PLATFORM_PREFIX}qplc_aggregates_16u,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}aggregates.cpp "\t${PLATFORM_PREFIX}qplc_aggregates_32u};\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}aggregates.cpp "}\n")

        #
        # Write mem_copy functions table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}memory_copy.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}memory_copy.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}memory_copy.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}memory_copy.cpp "memory_copy_table_t ${PLATFORM_PREFIX}memory_copy_table = {\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}memory_copy.cpp "\t${PLATFORM_PREFIX}qplc_copy_8u,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}memory_copy.cpp "\t${PLATFORM_PREFIX}qplc_copy_16u,\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}memory_copy.cpp "\t${PLATFORM_PREFIX}qplc_copy_32u};\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}memory_copy.cpp "}\n")

        #
        # Write mem_copy functions table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}zero.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}zero.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}zero.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}zero.cpp "zero_table_t ${PLATFORM_PREFIX}zero_table = {\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}zero.cpp "\t${PLATFORM_PREFIX}qplc_zero_8u};\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}zero.cpp "}\n")

        #
        # Write move functions table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}move.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}move.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}move.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}move.cpp "move_table_t ${PLATFORM_PREFIX}move_table = {\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}move.cpp "\t${PLATFORM_PREFIX}qplc_move_8u};\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}move.cpp "}\n")

        #
        # Write crc64 function table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}crc64.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}crc64.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}crc64.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}crc64.cpp "crc64_table_t ${PLATFORM_PREFIX}crc64_table = {\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}crc64.cpp "\t${PLATFORM_PREFIX}qplc_crc64};\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}crc64.cpp "}\n")

        #
        # Write xor_checksum function table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}xor_checksum.cpp "#include \"qplc_api.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}xor_checksum.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}xor_checksum.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}xor_checksum.cpp "xor_checksum_table_t ${PLATFORM_PREFIX}xor_checksum_table = {\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}xor_checksum.cpp "\t${PLATFORM_PREFIX}qplc_xor_checksum_8u};\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}xor_checksum.cpp "}\n")

        #
        # Write deflate functions table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}deflate.cpp "#include \"deflate_slow_icf.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate.cpp "#include \"deflate_hash_table.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate.cpp "#include \"deflate_histogram.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate.cpp "deflate_table_t ${PLATFORM_PREFIX}deflate_table = {\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate.cpp "\t reinterpret_cast<void *>(&${PLATFORM_PREFIX}slow_deflate_icf_body),\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate.cpp "\t reinterpret_cast<void *>(&${PLATFORM_PREFIX}deflate_histogram_reset),\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate.cpp "\t reinterpret_cast<void *>(&${PLATFORM_PREFIX}deflate_hash_table_reset)};\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate.cpp "}\n")

        #
        # Write deflate fix functions table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}deflate_fix.cpp "#include \"deflate_slow.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate_fix.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate_fix.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate_fix.cpp "deflate_fix_table_t ${PLATFORM_PREFIX}deflate_fix_table = {\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate_fix.cpp "\t reinterpret_cast<void *>(&${PLATFORM_PREFIX}slow_deflate_body)};\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}deflate_fix.cpp "}\n")

        #
        # Write setup_dictionary functions table
        #
        file(WRITE ${directory}/${PLATFORM_PREFIX}setup_dictionary.cpp "#include \"deflate_slow_utils.h\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}setup_dictionary.cpp "#include \"dispatcher/dispatcher.hpp\"\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}setup_dictionary.cpp "namespace qpl::core_sw::dispatcher\n{\n")
        file(APPEND ${directory}/${PLATFORM_PREFIX}setup_dictionary.cpp "setup_dictionary_table_t ${PLATFORM_PREFIX}setup_dictionary_table = {\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}setup_dictionary.cpp "\t reinterpret_cast<void *>(&${PLATFORM_PREFIX}setup_dictionary)};\n")

        file(APPEND ${directory}/${PLATFORM_PREFIX}setup_dictionary.cpp "}\n")

    endforeach()
endfunction()
