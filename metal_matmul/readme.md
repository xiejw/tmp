# vim: ft=help

benchmark >
  Pytorch          6.247    TFLOPS  ( 0.1763 secs)

  // element matmul
  k0 naive alg     0.0381   TFLOPS  (28.8648 secs)
  k1 shared cache  0.7384   TFLOPS  ( 1.469  secs)
  k1 manual unroll 0.9479   TFLOPS  ( 1.16   secs)
  k2 1d blocktile  2.0376   TFLOPS  ( 0.5396 secs)
  k3 2d blocktile  2.0384   TFLOPS  ( 0.5394 secs) (reg split after GROUPS >= 16)

  // simdgroup
  k4 tiling        0.2449   TFLOPS  ( 4.4888 secs)
  k5 2d(32) tile   2.4401   TFLOPS  ( 0.4506 secs)
     int index     2.778            ( 0.3957 secs)
     vec load      3.0610   TFLOPS  ( 0.3592 secs)

Refs:
- https://siboehm.com/articles/22/CUDA-MMM

Questions:
- https://stackoverflow.com/questions/77029896/how-to-detect-invalid-thread-group-setting-in-macos-metal-gpu

TODO clean k1 k0
- k0 tiling (not blocktile)

Metal undocumented
- unknown how to profile
- unknown when spill happen (NO PTX)

clang(metal) does not do good job
- cannot resolve index for array. must static unroll. not even x * C + y formular
- template based unroll has overhead.
