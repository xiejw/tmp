## Go cgo

The purpose of the test here is to understand how `CFLAGS` is handled in the
package level and global level.

### `make run`

Any package level code is compiled as a unit. It means

- If any `c` or `go` file (in the package) got changed, it would trigger the
  recompilation.

- If any `go` has `import C`, then `cgo` will be invoked with all `CFLAGS` in
  all the `go` files (in the package) concatenated together.

### `make run_with_prompt`

The `CGO_CFLAGS` will affect the whole project run with `go build` or `go run`.
It is passed to the underlying `c` code as flag during compilation as expected.

