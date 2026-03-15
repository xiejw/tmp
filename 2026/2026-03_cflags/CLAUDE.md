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

---

## Design: `flagsc` — DSL-to-C++ Flag Compiler

`flagsc` reads a `.flags` DSL file and code-generates a typed C++ header +
source. The generated files are statically typed with no runtime DSL parsing.

### Pipeline

```
.flags text
    │
    ▼
FlagDefParseDsl()        src/dsl.cc
    │  two-state machine: OUTSIDE → INSIDE → OUTSIDE
    │  produces FlagDef (array of FlagSpec)
    ▼
CodegenWrite()           src/codegen.cc
    │  iterates FlagSpec[], emits fprintf calls
    ▼
<basename>.h + <basename>.cc
```

### Key Data Structures (`src/dsl.h`)

```
FlagSpec          one flag: name, type, required, has_default, default_str
FlagDef           array of up to 64 FlagSpec + count
FlagType          STRING=0, INT=1, LONG=2
```

`default_str` holds the raw parsed value (quotes stripped for strings,
numeric text for int/long). The code generator emits it verbatim as a C++
literal.

### DSL Format

```
<name> {
  type:          string | int | long
  required:      true | false        (optional, default false)
  default_value: <value>             (optional; type must precede it)
}
```

### Generated API (in `<basename>.h` / `<basename>.cc`)

```cpp
struct Flags { /* one field + bool _set per flag */ };
void FlagsInit    (Flags *flags);
bool FlagsParse   (Flags *flags, int argc, char **argv, std::string *err_msg);
bool FlagsValidate(const Flags *flags, std::string *err_msg);
```

- `FlagsInit`     — zero/default initialises all fields
- `FlagsParse`    — iterates argv, dispatches per flag, validates int/long
- `FlagsValidate` — checks all `required` flags were provided

### Source Layout

| Path | Role |
|------|------|
| `src/dsl.h` + `src/dsl.cc`         | DSL parser |
| `src/codegen.h` + `src/codegen.cc` | Code generator |
| `cmd/flagsc.cc`                     | CLI entry point |
| `test.flags`                        | Test DSL input |
| `test_driver.cc`                    | Test driver compiled against generated files |
