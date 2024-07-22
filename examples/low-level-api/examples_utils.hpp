/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_EXAMPLES_UTILS_HPP_
#define QPL_EXAMPLES_UTILS_HPP_

#include <iostream>
#include <string>
#include <vector>

#include "qpl/qpl.h"

static inline int parse_execution_path(int argc, char** argv, qpl_path_t* path_ptr, int extra_arg = 0) {
    // Get path from input argument
    if (extra_arg == 0) {
        if (argc < 2) {
            std::cout << "Missing the execution path as the first parameter. Use either hardware_path or software_path."
                      << std::endl;
            return 1;
        }
    } else {
        if (argc < 3) {
            std::cout
                    << "Missing the execution path as the first parameter and/or the dataset path as the second parameter."
                    << std::endl;
            return 1;
        }
    }

    const std::string path = argv[1];
    if (path == "hardware_path") {
        *path_ptr = qpl_path_hardware;
        std::cout << "The example will be run on the hardware path." << std::endl;
    } else if (path == "software_path") {
        *path_ptr = qpl_path_software;
        std::cout << "The example will be run on the software path." << std::endl;
    } else if (path == "auto_path") {
        *path_ptr = qpl_path_auto;
        std::cout << "The example will be run on the auto path." << std::endl;
    } else {
        std::cout << "Unrecognized value for parameter. Use hardware_path, software_path or auto_path." << std::endl;
        return 1;
    }

    return 0;
}

#endif // QPL_EXAMPLES_UTILS_HPP_
