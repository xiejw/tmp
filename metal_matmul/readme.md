upper bound is 3Tflops(0.176301 secs)
k2 1d blocktile 1Tflops
k3 2d blocktile (reg split after GROUPS >= 8)

https://siboehm.com/articles/22/CUDA-MMM

https://stackoverflow.com/questions/77029896/how-to-detect-invalid-thread-group-setting-in-macos-metal-gpu

TODO clean k1 k0
- k0 naive alg
- k0 tiling (not blocktile)

clang(metal) does not do good job
- cannot resolve index for array. must static unroll. not even x * C + y formular
- template based unroll has overhead.
