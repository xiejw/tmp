# C++ Projects — Coding Style & Conventions

Applies to all C++ projects under this directory.

---

## Naming

- If not specified, all data structures are in `forge` namespace.
  - **Class methods**: CamelCase style with `verb_noun`, - e.g., `ScheduleWork`.
  - **Free functions** (no implicit "self"): `verb_noun` — e.g., `ConvertHtml`, `ParseToken`.
  - **Object-like functions** (first arg is the owning struct): `noun_verb` — e.g., `ErrInit`, `ErrEmit`.
- **Internal / static** symbols: `snake_case`, no prefix.
- Constants `kConstant`.
- Macros: `UPPER_SNAKE_CASE`.

## Code Style

- Never use raw new/delete or malloc/free
- Prefer raw pointers for function inputs if the lifetime at callsite is longer
  than the function.
- Prefer std::vector, std::string over raw arrays
- Use std::unique_ptr for ownership if needed. Or prefer value in lexical scope.
- Avoid std::shared_ptr unless necessary
- Use RAII for all resources
- Keep code simple and readable (no heavy templates)

## Makefile

- Standard: **C++17** (`-std=c++17`).
- Compiler is `${CXX}`.
- All c++ files are `.cc` suffix.
- Compiler flags: `-Wall -Werror -pedantic -Wextra -Wfatal-errors -Wconversion`.
- In Makefile, if `RELEASE` is defined, compiler flags should have
  `-O2 -DNDEBUG -march=native -flto -ffast-math` as well.
- Each logical concern gets its own function. No monolithic functions.
- No third-party libraries — stdlib only unless the project explicitly states otherwise.
- One blank line between top-level definitions. Section banners for logical groups:
  ```c++
  // === --- Section name ----------------------------------------------- ===
  //
  ```
- All produced assets must be in dir `.build`.

## Constants and Magic Numbers

All constants should be defined at top level to avoid magic numbers. And for
fixed size array, it is better to check the input length to report error.

## Error Handling

All fallible functions use this pattern:

```c++
int Foo(..., std::string *err_msg);
```
where return `0` on success, `-1` on error for most cases. The `err_msg` is used
to store the error messages.
