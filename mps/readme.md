## Linear Alg on MPS

The purpose of this experiment is to benchmark the performance of the different
backends (cpu/ane/mps) with different implementations
(pytorch/libtorch/Accelerate).

See https://github.com/xiejw/y/blob/main/doc/sop.md for set up.

### Benchmark PyTorch on macOS

This test is checking the performance with pytorch (arm64 + mps)

```
# read matmul_torch.py

pytorch cpu
c.matmul takes 0.6422574043273925

pytorch mps
c.matmul takes 0.17642641067504883
```

### Benchmark Apple Neural Engine on macOS

This test is checking the performance with Apple Neural Engine (aka `ane`),
provided via the `Accelerate` framework.

```
# read matmul_ane.cc

# make ane
# ./demo
c.matmul takes 0.608549
```

### Benchmark LibTorch on macOS

This test is checking the performance with pytorch (arm64 + mps)

```
libtorch mps
c.matmul takes 0.176301

libtorch cpu
c.matmul takes 0.635697
```

