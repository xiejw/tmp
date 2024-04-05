The Tutorial
============

Simple Example with path.
-------------------------

In the following example `@` is path lookup. Its namespace is different. Path is
separated by `.`.

All other names are `tensor`s. `Path` leaves be tensors as well.
```
let a = @a;
let b = @a;
let d = add(add(a, b) + @b.c);
print(c);
```

Function definition with path
-----------------------------
Fun definition can contain path as well. The arg must be `@`. Inside the func,
global `@` is not visible anymore.

For example, the example above can be rewritten as the following.
```
fn add3(a, b, c: @): {
  return add(add(a, b), @c.c)
}
let a = @a;
let b = @a;
let d = add3(a, b, @b);
print(d);
```

Notable
1. `: @` is a type annotation indicating `c` is a `path`. Default is `tensor`.
2. `:` at the end of the function definition `add3` is also a type annotation
   indicating the result type is a single `tensor`. This is common case.
