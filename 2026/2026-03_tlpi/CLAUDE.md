## Conventions

- Namespace: `forge`
- `#pragma once`, `snake_case`, C++17
- CXXFLAGS: `-Wall -Werror -pedantic -Wextra -fno-rtti -fno-exceptions`
- Cmd in folder `cmd` and all headers and source files are in folder `src`.
- Function naming: `TypeOperation()`
- Each `cmd/` file must begin with a usage comment block at the top (before includes).

## Coding Style

This codebase uses a C++11-like style that leans heavily toward C:

- **Raw pointers everywhere.** Pass raw pointers, not references. Move semantics
  and `std::move` are avoided. Ownership is documented via comments
  (`/* Owned */`, `/* Unowned */`, `/*moved_in*/`).
- **Simple classes/structs.** Classes are fine but must be deadly simple, close
  to C style — simple methods, no inheritance, no virtual methods.
- **No templates** unless absolutely necessary. Avoid heavy template-based `std`
  classes (e.g., prefer raw arrays over `std::vector`, `printf` over
  `std::cout`/`std::string` streams).
- **No exceptions, no RTTI.** Enforced by `-fno-exceptions -fno-rtti`.
- **Error handling** follows a consistent pattern:
  `bool Foo(args..., std::string *err_msg)` — returns `false` on success,
  `true` on error (with message written to `err_msg`).
- **Free functions**: if the class methods is complicated, define helper methods
  as free functions taking arguments as explicit inputs so it is super clear
  dependencies.
