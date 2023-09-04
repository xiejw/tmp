# vim: ft=help

benchmark >
  Pytorch          4.674    TFLOPS  ( 0.1763 secs)

  // element matmul
  k0 naive alg     0.019049 TFLOPS  (28.8648 secs)
  k1 shared cache  0.3742   TFLOPS  ( 1.469  secs)
  k1 manual unroll 0.4740   TFLOPS  ( 1.16   secs)
  k2 1d blocktile  1.0188   TFLOPS  ( 0.5396 secs)
  k3 2d blocktile  1.0192   TFLOPS  ( 0.5394 secs) (reg split after GROUPS >= 16)

  // simdgroup
  k4 tiling        0.1225   TFLOPS  ( 4.4888 secs)

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
