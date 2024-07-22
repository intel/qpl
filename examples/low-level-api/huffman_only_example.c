/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_HUFFMAN_ONLY_EXAMPLE] */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qpl/qpl.h"

/**
 * @brief This example requires a command line argument to set the execution path. Valid values are `software_path`
 * and `hardware_path`.
 * In QPL, @ref qpl_path_software (`Software Path`) means that computations will be done with CPU.
 * Accelerator can be used instead of CPU. In this case, @ref qpl_path_hardware (`Hardware Path`) must be specified.
 * If there is no difference where calculations should be done, @ref qpl_path_auto (`Auto Path`) can be used to allow
 * the library to chose the path to execute. The Auto Path usage is not demonstrated by this example.
 *
 * @warning ---! Important !---
 * `Hardware Path` doesn't support all features declared for `Software Path`
 *
 * The C-based example demonstrates data compression/decompression using 2 separate jobs with Huffman-only encoding.
 */
#define source_size 1000U

int main(int argc, char** argv) {
    printf("Intel(R) Query Processing Library version is %s.\n", qpl_get_library_version());

    // Default to Software Path
    qpl_path_t execution_path = qpl_path_software;

    if (argc < 2) {
        printf("Parameter for execution path was not provided. Use hardware_path or software_path.\n");
        return 1;
    }

    // Get path from input argument
    if (strcmp(argv[1], "hardware_path") == 0) {
        execution_path = qpl_path_hardware;
        printf("The example will be run on the hardware path.\n");
    } else if (strcmp(argv[1], "software_path") == 0) {
        execution_path = qpl_path_software;
        printf("The example will be run on the software path.\n");
    } else if (strcmp(argv[1], "auto_path") == 0) {
        execution_path = qpl_path_auto;
        printf("The example will be run on the auto path.\n");
    } else {
        printf("argv[1] = %s", argv[1]);
        printf("Unrecognized value for execution path parameter. Use hardware_path, software_path or auto_path.\n");
        return 1;
    }

    // Source and output containers
    uint8_t source[source_size];
    uint8_t destination[source_size * 2];
    uint8_t reference[source_size];

    uint32_t size = 0;

    // Getting job size
    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) {
        printf("An error acquired during job size getting. Error status = %d\n", status);
        return status;
    }

    qpl_job* compress_job = NULL;
    compress_job          = (qpl_job*)malloc(size);
    if (compress_job == NULL) {
        printf("An error acquired during allocation of compression job. Error status = %d\n", status);
        return status;
    }

    status = qpl_init_job(execution_path, compress_job);
    if (status != QPL_STS_OK) {
        printf("An error acquired during job initializing. Error status = %d\n", status);

        // Since qpl_init_job allocates and initialize internal structures,
        // it is required to call qpl_fini_job in case of an error to free all internal resources.
        qpl_fini_job(compress_job);
        free(compress_job);

        return status;
    }

    // Allocating the compression Huffman Table object for Huffman-only
    qpl_huffman_table_t c_huffman_table = NULL;

    // The next line is a workaround for DEFAULT_ALLOCATOR_C macros.
    // This macros works only with C++ code.
    allocator_t default_allocator_c = {malloc, free};
    status = qpl_huffman_only_table_create(compression_table_type, execution_path, default_allocator_c,
                                           &c_huffman_table);

    if (status != QPL_STS_OK) {
        printf("An error acquired during huffman table creation. Error status = %d\n", status);

        qpl_huffman_table_destroy(c_huffman_table);
        qpl_fini_job(compress_job);
        free(compress_job);

        return status;
    }

    // Initializing qpl_job structure before performing a compression operation.
    compress_job->op            = qpl_op_compress;
    compress_job->next_in_ptr   = source;
    compress_job->next_out_ptr  = destination;
    compress_job->available_in  = source_size;
    compress_job->available_out = (uint32_t)(source_size * 2);
    compress_job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_NO_HDRS | QPL_FLAG_GEN_LITERALS |
                          QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;
    compress_job->huffman_table = c_huffman_table;

    // Executing compression operation
    status = qpl_execute_job(compress_job);
    if (status != QPL_STS_OK) {
        printf("Error during compression occurred. Error status = %d\n", status);

        qpl_huffman_table_destroy(c_huffman_table);
        qpl_fini_job(compress_job);
        free(compress_job);

        return status;
    }

    const uint32_t last_bit_offset = compress_job->last_bit_offset;
    const uint32_t compressed_size = compress_job->total_out;

    // Freeing compression job resources
    status = qpl_fini_job(compress_job);
    if (status != QPL_STS_OK) {
        printf("An error acquired during compression job finalization. Error status = %d\n", status);

        free(compress_job);
        qpl_huffman_table_destroy(c_huffman_table);

        return status;
    }
    free(compress_job);

    // The code below checks if a compression operation works correctly

    // Allocating the decompression Huffman Table object for Huffman-only
    qpl_huffman_table_t d_huffman_table = NULL;
    status = qpl_huffman_only_table_create(decompression_table_type, execution_path, default_allocator_c,
                                           &d_huffman_table);

    if (status != QPL_STS_OK) {
        printf("An error acquired during decompression Huffman table creation. Error status = %d\n", status);

        qpl_huffman_table_destroy(c_huffman_table);
        qpl_huffman_table_destroy(d_huffman_table);

        return status;
    }

    // Initializing decompression table with the values from compression table
    status = qpl_huffman_table_init_with_other(d_huffman_table, c_huffman_table);
    if (status != QPL_STS_OK) {
        printf("An error acquired during decompression Huffman table initialization failed. Error status = %d\n",
               status);

        qpl_huffman_table_destroy(c_huffman_table);
        qpl_huffman_table_destroy(d_huffman_table);

        return status;
    }

    // Destroying compression huffman_table
    status = qpl_huffman_table_destroy(c_huffman_table);
    if (status != QPL_STS_OK) {
        printf("An error acquired during Huffman table destroying. Error status = %d\n", status);

        qpl_huffman_table_destroy(d_huffman_table);

        return status;
    }

    qpl_job* decompress_job = NULL;
    decompress_job          = (qpl_job*)malloc(size);
    if (decompress_job == NULL) {
        printf("An error acquired during malloc function for decompress job. Error status = %d\n", status);
        return status;
    }

    status = qpl_init_job(execution_path, decompress_job);
    if (status != QPL_STS_OK) {
        printf("An error acquired during compression job initializing. Error status = %d\n", status);

        qpl_huffman_table_destroy(d_huffman_table);
        qpl_fini_job(decompress_job);
        free(decompress_job);

        return status;
    }

    // Initializing decompression qpl_job structure before performing a decompression operation
    decompress_job->op              = qpl_op_decompress;
    decompress_job->next_in_ptr     = destination;
    decompress_job->next_out_ptr    = reference;
    decompress_job->available_in    = compressed_size;
    decompress_job->available_out   = (uint32_t)(source_size);
    decompress_job->ignore_end_bits = (8 - last_bit_offset) & 7;
    decompress_job->flags           = QPL_FLAG_NO_HDRS | QPL_FLAG_FIRST | QPL_FLAG_LAST;
    decompress_job->huffman_table   = d_huffman_table;

    // Executing decompression operation
    status = qpl_execute_job(decompress_job);
    if (status != QPL_STS_OK) {
        printf("Error during decompression occurred. Error status = %d\n", status);

        qpl_huffman_table_destroy(d_huffman_table);
        qpl_fini_job(decompress_job);
        free(decompress_job);

        return status;
    }

    // Freeing decompression job resources
    status = qpl_fini_job(decompress_job);
    if (status != QPL_STS_OK) {
        printf("An error acquired during decompression job finalization. Error status = %d\n", status);

        qpl_huffman_table_destroy(d_huffman_table);
        free(decompress_job);

        return status;
    }
    free(decompress_job);

    // Destroying decompression huffman_table
    status = qpl_huffman_table_destroy(d_huffman_table);
    if (status != QPL_STS_OK) {
        printf("An error acquired during decompression Huffman table destroying. Error status = %d\n", status);
        return status;
    }

    // Compare reference functions
    for (size_t i = 0; i < source_size; i++) {
        if (source[i] != reference[i]) {
            printf("Content wasn't successfully compressed and decompressed.");
            return -1;
        }
    }

    printf("Content was successfully compressed and decompressed.\n");
    printf("Compressed size: %d\n", compressed_size);

    return 0;
}

//* [QPL_LOW_LEVEL_HUFFMAN_ONLY_EXAMPLE] */
