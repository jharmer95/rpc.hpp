# rpc.hpp Security Policy

This policy describes the ways in which security vulnerabilities may be reported.

Please keep in mind:

- Security vulnerabilities of a particular client/server implementation are not covered.
This is up to the author(s) of that implementation.
- Likewise, vulnerabilities of serialization libraries are not covered, though the adapters provided
for them are.
- The examples, tests, and benchmarks for this library are also not covered. These tests and
examples are not designed for production use.

## Supported Versions

| Version     | Supported          |
|-------------|--------------------|
| 1.0.x       | :white_check_mark: |
| 1.1-rc.X    | :white_check_mark: |
| < 1.0       | :x:                |
| X.X-alpha.X | :x:                |
| X.X-beta.X  | :x:                |

## Reporting a Vulnerability

Please report vulnerabilities directly to jharmer95@gmail.com using the following format:

- Put "RPC.HPP SECURITY VULNERABILITY: <brief_description_of_issue>" as the subject of the email.
- In the body, put as detailed a description as you can, including the steps to reproduce and the
version(s) affected.
- If your preferred method of contact is **NOT** the email address you used to send the report,
please also provide that in the body as well.
- Please also indicate if you know the fix and, if possible, attach an updated archive or patch
file(s) with these changes.

You will be contacted with updates regarding this vulnerability as it is received, addressed,
dismissed, or fixed in production.

In the meantime, please avoid disclosing this to the public or creating a bug report/pull request.
You may be asked for additional details or modifications.

If you are providing a fix, you will be contacted to create a pull request to merge the change into
production. If you do not wish to be involved in this process or would like to remain anonymous,
please indicate that in the correspondence.
