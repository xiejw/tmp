## Linear Alg on Metal/MPS

The purpose of this experiment is to benchmark the performance of the different
backends (cpu/ane/mps) with different implementations
(pytorch/libtorch/Accelerate).

See https://github.com/xiejw/y/blob/main/doc/sop.md#libtorch for set up.


Some lessons:
- MPS is a  high level framework. It is not easy to undersand nor customize.
- This means libtorch is not a good fit.

**TODO**: the metal shader version got started. It is developed under the `metal`
folder and not yet done. The vec add is done.

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
# read matmul_torch.cc

# make torch
# ./demo

libtorch mps
c.matmul takes 0.176301

libtorch cpu
c.matmul takes 0.635697
```

