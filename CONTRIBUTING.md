# How to Contribute

## Code of Conduct

This project and everyone participating in it is governed by the
[rpc.hpp Code of Conduct](CODE_OF_CONDUCT.md).
By participating, you are expected to uphold this code.\
Please report unacceptable behavior to jharmer95@gmail.com.

## Reporting an Issue

You may report an issue from the [issues](https://github.com/jharmer95/rpc.hpp/issues) page.

Before submitting a new issue, please go through the following steps:

1. Check if you are running the latest version
  - The easiest way to do this is to open the file `rcp.hpp` on your system
  - The version number will be in a comment near the top of the file
  - Compare that to the [latest release](https://github.com/jharmer95/rpc.hpp/releases/latest)
  - If your version is out of date, please update to the newest version before reporting an issue
2. Perform a search to see if a similar issue has already been opened
  - If your issue is the same, give that issue a :+1:
    - **Please don't add additional comments like _"Me too"_ or _"I need this fix"_**,
    a :+1: conveys this much more succinctly and allows for an easy way to visualize the
    popularity/demand for this issue.
  - Even if your issue is not _exactly_ the same, it may be more beneficial to provide some extra
    context or details for your situation via a comment vs. a new issue.
    - Sometimes it's easier to understand the scope of an issue this way and can allow for one patch
    to be made instead of multiple

Please stick to using one of the provided templates to ensure that issues are consistent.

### Feature Requests

Using the Feature Request template, make sure to replace the comments with the indicated content.
Hopefully, this can be replaced by GitHub Issue Forms soon.

An example feature request might look like:

```markdown
# Feature Request: Add Support For SomeSerialLibrary

## Brief Summary of Request
<!-- insert a brief summary of your feature request here -->
Add adapter to support for serialization using
[SomeSerialLibrary](https://github.com/fake-author/some-serial-lib)

## What Is The Value Added With This Feature
<!-- provide some reasons why you believe this feature is valuable -->
SomeSerialLibrary is a really powerful serialization library and an adapter for
it would be of great benefit to the community.

Some features of the library are:

- Feature A
- Feature B

## Possible Implementation
<!-- if you have some ideas on how this can actually be done put them here -->
- Create an additional header: `rpc_adapters/rpc_some_serial_lib.hpp`

## Desired Outcome
<!-- describe the outcome you'd like to see here as well as provide some
     code/pseudocode to indicate the usage or API of your request -->

    ```C++
    class MyClient : public rpc::client_interface<some_serial_lib_adapter>
    {
    };

    // ...

    auto result = MyClient.call_func("SomeFunc");
    ```

## Other Details
<!-- feel free to put any other details here -->
```

### Bug Reports

Using the Bug Report template, make sure to replace the comments with the indicated content.
Hopefully, this can be replaced by GitHub Issue Forms soon.

> **NOTE:** If you believe that an issue is most likely a security concern/vulnerability,
> **DO NOT** file a bug report.
>
> Please see the [security policy](SECURITY.md) on how to report this!

An example bug report might look like:

```markdown
# Bug Report: rapidjson adapter fails to compile with Clang 12

## System Information

- Platform: x86_64
- Operating System: Ubuntu 21.10
- Compiler: clang 12.0.1

## Brief Summary of Bug
<!-- here you can provide a brief summary of the bug -->
When compiling with Clang 12.0.1, the error:
"This fake error occurred (rpc_adapters/rpc_rapidjson.hpp:80)" is given.

- Issue does not exist in gcc.

## Steps To Reproduce
<!-- please list the **exact** steps taken to encounter this bug or the cases
     when it occurs -->
Using example code, compile with Clang 12 or newer with `-D RPC_HPP_ENABLE_RAPIDJSON`

## Areas Of Concern
<!-- what parts of the project are impacted by this bug, everything? and
     experimental feature? only a few functions? -->
- Prevents using rapidjson with the newer versions of Clang.
- Requires switching to gcc or an older version of Clang.

## Estimated Severity
<!-- Please "check" one of the boxes by replacing the blank space in the square
     brackets with an X -->

- [ ] Pedantic (code-style, design)
- [ ] Minimal (compiler warnings, legacy support)
- [ ] Minor (incompatibilities, problems that have easy work-arounds)
- [X] Major (breaking changes, problems with more advanced work-arounds)
- [ ] Critical (major security issues, complete failure)

## Possible Fixes
<!-- if you know of a way that could possibly fix this issue, list it here,
     feel free to fork and submit a PR once this bug report is submitted -->
```

## Forking the Project

To make changes to the project, you should first fork the project on GitHub. This makes it easy to
work on your own copy of the project without having to worry about creating a lot of branches. You
may work on the `main` branch of your fork, though it is recommended that you create a new branch
for each change you are trying to fix, that way, you can keep your copy of `main` up-to-date with
upstream. You may merge this branch back into your `main` branch prior to submitting a pull request.

## Contributing Code

`rpc.hpp` welcomes code contributions from anyone of any skill level or
background.

Contributors must agree to the [code of conduct](CODE_OF_CONDUCT.md) and
additionally, should follow the guidelines provided in this section to ensure
their changes are reviewed and accepted.

### Setting Up Your Build Environment

Please refer to the [Installation Guide](./INSTALL.md#setting-up-your-environment) for details
on getting your build environment set up and compiling the project and its tests.

### Running Tests

To build and run the tests, see [Running Tests](./INSTALL.md#running-tests) from the Installation
Guide.

NOTE: All tests must pass on CI for a PR to be accepted! These tests will be performed using both
GCC and Clang on Ubuntu Linux.

### Code Conventions

`rpc.hpp` aims to maintain a clean, easy to read, and idiomatic codebase. It does this by following
best practices such as those provided by:

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- Jason Turner's [cppbestpractices](https://github.com/lefticus/cppbestpractices)

#### Code Style

To simplify keeping a consistent code style, a `.clang-format` file is provided. Please make sure
you have clang-format installed and run it on any changed C++ files (`.h`, `.hpp`, `.cpp`, etc.)
before submitting a PR.

In addition to this, there are a few naming conventions to stick to:

##### Naming Convention

The naming convention is very similar to that of the C++ standard library:

- All variables should be meaningfully named unless they are trivial such as:
  - A loop index (`i`, `j`, `k`)
  - A loop iterator (`it`, `it2`, etc.)
  - A temporary variable being consumed within the next few lines, like for a swap (`tmp`)
  - An unrestricted generic template typename (`T`, `U`)
- Variables do not need to indicate their type
  - No hungarian notation (**no** `iSomeVar`, `fSomeVar`)
  - No type prefixes/suffixes (**no** `length_int`, `string_name`)
  - (Smart) pointers/references _may_ include a `ptr_`/`ref_` prefix when needed for clarity
- All variable/function/parameter/class/struct/namespace/member names should be `snake_case` with a
few exceptions:
  - `SCREAMING_SNAKE_CASE`:
    - Macros
    - Global variables
    - `constexpr` variables
  - `CamelCase`
    - `template` parameters
- `private`/`protected` non-static member variables should have a `m_` prefix
- `private`/`protected` static member variables should have a `s_` prefix
- Type aliases should have a `_t` suffix
- Boolean variables and boolean-returning functions should indicate what they are testing,
(ex. `is_full`, `has_value()`, `was_used`)
  - Booleans should only be named in the _affirmative_ (**no** `is_not_empty`, `no_value`)

#### Documentation Comments

All public variables, functions, namespaces, and types _must_ be documented using a Doxygen comment,
where "public" means anything in the `rpc` namespace and its child namespaces, excluding things in the `details` namespace
and `private`/`protected` members of classes/structs.

The comment follows the [Doxygen spec](https://www.doxygen.nl/manual/docblocks.html), using the
`///` style of comment.

An example comment would look like:

```C++
///@brief Calculates the length of a given string.
///
/// Iterates through the string until a null character ('\0') is found.
///@param str The string to find the length of.
///@return size_t The length of the string.
size_t GetStrLen(const char* str);
```

If documentation comments are added/changed/removed, the Doxygen docs should be re-generated. See
[Generating Doxygen Docs](#generating-doxygen-docs) below for more information.

## Contributing Documentation

Documentation exists in two forms:

- In-code documentation via Doxygen
- External Markdown documents

Both of these forms of documentation are important to ensure users and developers have an easy time
working with the library and therefore should stay as clean and up-to-date as possible.

### Generating Doxygen Docs

If the documentation comments have been altered, a new set of Doxygen docs should be generated.

You will need to make sure that the following components are installed on your machine:

- `doxygen`
- `graphviz`
- `dot`

NOTE: On some platforms, `dot` is installed as part of the `doxygen` package, and is not provided
separately. If you have `doxygen` installed, it is usually safe to assume that `dot` is also
installed.

Once you have these tools installed you may proceed with the documentation generation:

- First, empty the `docs` directory found under the project root directory.
- Then, configure the CMake project such that `GENERATE_DOXYGEN` and all of the adapters are turned
on.
  - You may turn off all other options.
- Next, run the command `cmake --build build --target docs` to generate the new documentation and
populate the `docs` directory
- Make sure to commit the changes to `docs`

## Committing Changes

When creating git commits, there are a few guidelines you should keep in mind:

- To the best of your ability, try to address only one issue or add only one feature per commit.
  - For example, a single commit should not "Fix bug #1234 and add cool new feature".
- When writing commit messages, use the present-tense and imperative mood to describe the change(s).
  - A good way to think about this is: if you added "This commit will" before your message, it would
  be accurate and form a proper sentence.
  - i.e. "Add logging" not "Adds logging" or "Added logging".
- Specific issues may be referenced by their number, ex: "... fixes #123".
  - When possible, please use a more detailed description of the issue or feature in the commit, you
  may refer to the issue number later, in the body of the message. This will make it easier to find
  where a specific change was made without having to reference the closed issue.
- If a commit message can not fit in a single, 50 character sentence, it should be broken up into
multiple lines with the following format:

```markdown
50 character or less summary

Separated by an empty line, detailed description of the changes made as
necessary, with lines limited to 72 characters.

* Bullet points
* Are
* Acceptable
```

## Submitting Pull Requests

To submit a pull request, make sure you are making your changes on a fork of the project. See
[Forking the Project](#forking-the-project) for more details. Once you have finalized your changes,
you may submit a pull request against your fork.

Please make sure to provide adequate comments
describing the changes made and the reasoning behind those changes. It really helps to have good
commit messages.

Also make sure to reference any issues that these changes may address (i.e. closes #123).

If you know there are breaking or backwards-incompatible changes, **_please_** indicate this in the
pull request comments and by tagging the PR with the `potentially-breaking` label.

### Acceptance Criteria

A few basic requirements will need to be made before _any_ PR is accepted:

- All automated CI checks complete successfully
- All new/modified source code files must be formatted with clang-format
- If there are any changes to documentation comments, new documentation must be generated
  - If new public functions/variables/types/namespaces are added, they must have documentation
  comments
- Example code must compile
- If the API changes, external documentation must be updated
- No new warnings may occur (must build with `-Werror`)

If all these conditions are met, the code will be reviewed. Once the code has been reviewed on of
the following may occur:

- :heavy_check_mark: The merge will be accepted, a new version will be generated, and you will be
notified.
  - You may delete your fork/development branch at this time if you like.
  - You will probably want to update your fork to pull in the changes from upstream.
- :warning: The merge will require some additional work or discussion, with constructive comments
provided.
  - After receiving the notification, you can make the necessary alterations and push those changes.
  to your fork. You will **not** need to submit a new PR, the push will update the existing PR
  automatically.
  - Once these new changes are received the review process will start over.
- :x: The merge will be rejected with a detailed reason as to why.
