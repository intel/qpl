<!--
Copyright (C) 2022 Intel Corporation
SPDX-License-Identifier: MIT
-->

Intel® Query Processing Library (Intel® QPL)
============================================

The Intel® Query Processing Library (Intel® QPL) is an open-source library to provide
high-performance query processing operations on Intel CPUs. Intel® QPL is aimed to support
capabilities of the new Intel® In-Memory Analytics Accelerator (Intel® IAA) available on Next
Generation Intel® Xeon® Scalable processors, codenamed Sapphire Rapids processor, such as very high
throughput compression and decompression combined with primitive analytic functions, as well as to
provide highly-optimized SW fallback on other Intel CPUs. Intel QPL primarily targets applications
such as big-data and in-memory analytic databases.

Intel QPL provides Low-Level C API. You can use it from C/C++ applications.
You can also find Java* bindings in the [qpl-java](https://github.com/intel/qpl-java) project.
Refer to its documentation for details.

## Table of Contents

- [Get Started](#get-started)
- [Documentation](#documentation)
- [Testing](#testing)
- [How to Contribute](#how-to-contribute)
- [How to Report Issues](#how-to-report-issues)
- [License](#license)
- [Security](#security)
- [Notices and Disclaimers](#notices-and-disclaimers)

## Get Started

To set up and build the Intel QPL,
refer to [Installation](https://intel.github.io/qpl/documentation/get_started_docs/installation.html) page.

## Documentation

Documentation is delivered using GitHub Pages. See full Intel QPL [online documentation](https://intel.github.io/qpl/index.html).
- [Developer Guide](https://intel.github.io/qpl/documentation/dev_guide_docs/low_level_developer_guide.html) explains key concepts of Low-Level C API.
- [Low-Level C API Reference](https://intel.github.io/qpl/documentation/dev_ref_docs/low_level_api.html) provides reference to key APIs of the library.

To build Intel QPL offline documentation, see the [Documentation Build Prerequisites](https://intel.github.io/qpl/documentation/get_started_docs/installation.html#building-the-documentation) chapter.

## Testing

See [Intel QPL Testing](https://intel.github.io/qpl/documentation/get_started_docs/testing.html) chapter for details about testing process.

## How to Contribute

See [Contributing document](CONTRIBUTING.md) for details about contribution process.

## How to Report Issues

See [Issue Reporting](https://intel.github.io/qpl/documentation/contributing_docs/issue_reporting.html) chapter for details about issue reporting process.

## License

The library is licensed under the MIT license. Refer to the
"[LICENSE](LICENSE)" file for the full license text.

This distribution includes third party software governed by separate license
terms (see "[THIRD-PARTY-PROGRAMS](third-party-programs.txt)").

## Security

For information on how to report a potential security issue or vulnerability see [Security Policy](SECURITY.md)

## Notices and Disclaimers

Intel technologies may require enabled hardware, software or service activation.
No product or component can be absolutely secure.
Your costs and results may vary.

© Intel Corporation. Intel, the Intel logo, and other Intel marks are trademarks of
Intel Corporation or its subsidiaries.
Other names and brands may be claimed as the property of others.

No license (express or implied, by estoppel or otherwise) to any intellectual
property rights is granted by this document.

The products described may contain design defects or errors known as errata
which may cause the product to deviate from published specifications.
Current characterized errata are available on request.

Microsoft, Windows, and the Windows logo are trademarks, or registered trademarks
of Microsoft Corporation in the United States and/or other countries.
Java is a registered trademark of Oracle and/or its affiliates.

\* Other names and brands may be claimed as the property of others.
