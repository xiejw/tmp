# Chapter 1 Implicit Integer/Sign/Float Conversion

Loss due to numeric conversion (int64 to int8) or sign changed (int8 to uint8)
is very dangerous.

While they are allowed by `c` spec, how to capture them with compilers?

## Integer Conversion
We could
take a look of the `c` code with `int` types only.

```c
#include <cstdint>

#define UNUSED(x) ((void)(x))

int main() {
  uint8_t  a = 1;
  int64_t  b = 23;

  int64_t  c = a + b;           // This line is fine.
  int8_t   d = a + b;           // Implicit int conversion
  uint64_t e = a + b;           // Sign conversion
  uint64_t f = int64_t(a + b);  // Sign conversion

  UNUSED( c );
  UNUSED( d );
  UNUSED( e );
  UNUSED( f );
  return 0;
}
```
To capture all potential issues `-Wconversion` is the key. We could see the
compiler output as follows
```bash
$ clang++ -std=c++23 -Wall -pedantic -Wextra -Werror main.cc  -Wconversion
main.cc:10:18: error: implicit conversion loses integer precision: 'int64_t' (aka 'long') to 'int8_t' (aka 'signed char') [-Werror,-Wimplicit-int-conversion]
   10 |   int8_t   d = a + b;           // Implicit int conversion
      |            ~   ~~^~~
main.cc:11:18: error: implicit conversion changes signedness: 'int64_t' (aka 'long') to 'uint64_t' (aka 'unsigned long') [-Werror,-Wsign-conversion]
   11 |   uint64_t e = a + b;           // Sign conversion
      |            ~   ~~^~~
main.cc:12:16: error: implicit conversion changes signedness: 'int64_t' (aka 'long') to 'uint64_t' (aka 'unsigned long') [-Werror,-Wsign-conversion]
   12 |   uint64_t f = int64_t(a + b);  // Sign conversion
      |            ~   ^~~~~~~~~~~~~~
3 errors generated.
```
Note that safe integer conversion is ok. No compiler flag (`-Weverything`) will
complain them.

## Float Conversion
If we do the similar testing for float conversions, this is the result
```c
// main.cc
#include <cstdint>

#define UNUSED(x) ((void)(x))

typedef float  f32_t;
typedef double f64_t;

int main() {
  uint8_t  a = 1;
  int64_t  b = 23;

  f32_t   c = a;  // This is fine
  f32_t   d = b;
  f64_t   e = b;
  f32_t   g = 23.0;
  int8_t  h = g + a;
  int64_t i = g + b;
  int64_t j = g + a;

  f64_t   k = 23.0;
  auto    l = g + k;  // This is fine.
  f32_t   m = g + k;

  UNUSED( c );
  UNUSED( d );
  UNUSED( e );
  UNUSED( h );
  UNUSED( i );
  UNUSED( j );
  UNUSED( l );
  UNUSED( m );
  return 0;
}
```

```bash
 clang++ -std=c++23 -Wall -pedantic -Wextra -Werror main2.cc  -Wconversion
main2.cc:13:15: error: implicit conversion from 'int64_t' (aka 'long') to 'f32_t' (aka 'float') may lose precision [-Werror,-Wimplicit-int-float-conversion]
   13 |   f32_t   d = b;
      |           ~   ^
main2.cc:14:15: error: implicit conversion from 'int64_t' (aka 'long') to 'f64_t' (aka 'double') may lose precision [-Werror,-Wimplicit-int-float-conversion]
   14 |   f64_t   e = b;
      |           ~   ^
main2.cc:16:17: error: implicit conversion turns floating-point number into integer: 'f32_t' (aka 'float') to 'int8_t' (aka 'signed char') [-Werror,-Wfloat-conversion]
   16 |   int8_t  h = g + a;
      |           ~   ~~^~~
main2.cc:17:17: error: implicit conversion turns floating-point number into integer: 'f32_t' (aka 'float') to 'int64_t' (aka 'long') [-Werror,-Wfloat-conversion]
   17 |   int64_t i = g + b;
      |           ~   ~~^~~
main2.cc:17:19: error: implicit conversion from 'int64_t' (aka 'long') to 'f32_t' (aka 'float') may lose precision [-Werror,-Wimplicit-int-float-conversion]
   17 |   int64_t i = g + b;
      |                 ~ ^
main2.cc:18:17: error: implicit conversion turns floating-point number into integer: 'f32_t' (aka 'float') to 'int64_t' (aka 'long') [-Werror,-Wfloat-conversion]
   18 |   int64_t j = g + a;
      |           ~   ~~^~~
main2.cc:22:17: error: implicit conversion loses floating-point precision: 'f64_t' (aka 'double') to 'f32_t' (aka 'float') [-Werror,-Wimplicit-float-conversion]
   22 |   f32_t   m = g + k;
      |           ~   ~~^~~
7 errors generated.
```
