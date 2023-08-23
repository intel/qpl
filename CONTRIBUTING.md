<!--
Copyright (C) 2023 Intel Corporation
SPDX-License-Identifier: MIT
-->

Intel® Query Processing Library (Intel® QPL) Contribution Rules
===============================================================

## How to Contribute

We welcome community contributions to Intel QPL! 

If you have an idea how to improve the product:

 - Let us know via [GitHub* Issues](https://github.com/intel/qpl/issues). See more in [Issues reporting](https://intel.github.io/qpl/documentation/contributing_docs/issue_reporting.html).
 - Send your proposal directly via [pull request](https://github.com/intel/qpl/pulls).



## Intel QPL repository scheme

Intel QPL supports two repositories that are named as "Public Repository" and "Mirror". Privately hosted "Mirror" is required for extensive internal testing and experimental features development. 

Existing automation guarantees the same commits history for both repositories.

<pre>
  +-------------+                     +------------+ 
  | Public Repo |                     |   Mirror   |
  | ----------- |   synchronization   |   -------  |       +----------+
  |    rev.1    | <-----------------> |    rev.1   |       | internal |
  |    rev.2    |   of each commit    |    rev.2   |-----> | testing  |
  |    rev.3    |                     |    rev.3   |       +----------+
  +-------------+                     +------------+
</pre>  

## Contribution Flow

- Contributor creates fork from develop, commits the changes into the created branch, opens a PR and requests a review. 
- Intel QPL repository maintainer must ensure that the code is safe for internal execution, get code into "Mirror" and run private testing.
- Contributor applies feedback provided by Intel QPL repository maintainer in opened PR.
- Intel QPL repository maintainer merges  the changes "as is" from "Mirror", when private testing is passed.
> **Note**
> Original PR is closed because merging PRs on the external GitHub repo isn't supported.

- Merging into "Mirror" triggers synchronization and makes contributions available in the "Public" repository.

<pre>
  
  +-----------------+                     
  |   Public Repo   | 
  |   -----------   |                     +---------------------+
  |   PR is open    |                     |       Mirror        |
  |       |         | PR is cherry-picked |       ------        |
  |       |---------|---------------------|--> Heavy private    |
  |       |         |     to "Mirror"     |       testing       |
  |  Code Review    |                     |          |          |
  |   has passed    |                     |          |          | 
  |       |         |  synchronization    |          |          | 
  |       |---------|---------------------|-> Cherry-picked PR  |        
  |       |         |      point          |       is merged     |
  |  PR is closed   |                     |      to "Mirror"    |
  +-----------------+                     +---------------------+
</pre>

## Pull Request Checklist
Before sending your pull requests, ensure that:
 - Intel QPL builds successfully with proposed changes.
 - Changes don't bring new failed functional tests. List of failed tests are listed in latest Release Notes. Read more about functional tests in [Testing](https://intel.github.io/qpl/documentation/get_started_docs/testing.html).
 - For new APIs "algorithmic", "bad arguments" and "negative" tests are implemented and passed. Read more about type of functional tests in [Functional tests](https://intel.github.io/qpl/documentation/get_started_docs/testing.html#functional-tests).

### Documentation Guidelines

Intel QPL uses Doxygen for inline comments in public header files that are used to build the API reference. Use [documentation instructions](README.md#documentation) to generate full documentation from sources with Doxygen.
