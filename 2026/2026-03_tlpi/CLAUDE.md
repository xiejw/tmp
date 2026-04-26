## Conventions

- Follow @lang_cc.md.
- Namespace: `forge`
- `#pragma once`
- Extra compiler flags (project-specific): `-fno-rtti -fno-exceptions`
- Cmd in folder `cmd` and all headers and source files are in folder `src`.
- Each `cmd/` file must begin with a usage comment block at the top (before includes).
  The block must follow this exact format:
  ```
  // cmd/<filename>.cc
  // Usage: <binary> <args...>
  //   <arg>  description of argument
  //   ...
  //   <one-line description of what the program does>
  //
  // Example:
  //   terminal 1: .build/<binary> <example args>
  //   terminal 2: .build/<binary> <example args>   # or expected output / next step
  ```
  The last line before the example is a prose sentence describing what the program does.
  The example section shows a concrete invocation (or sequence for multi-process programs).
