## Coding Style

- Simple and easy to read.

### Layout

- The `main.go` should be in `cmd` folder.
- All other supporting files should be in `src` folder.
- The outout dir is `.build`.

### Makefile

- All `Makefile` should have `compile`, `release`, `clean`, `test` and `fmt`.
- If no tests to run, `test` should be dependent on `compile`
- If no special actions for `release`, `release` should be dependent on `compile`
- `fmt` should format the code entirely, with `go fmt <package_name>/...`.
- `clean` just remove the `.build`.
