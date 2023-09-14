

```
Tota size 64MiB
M1 MAX 400 GBit/s for Unified Memory

one simd group
0.195593 secs  0.320 GiB/s each lane copy one element each time
0.067644 secs each lane copy 4 elements each time (vec load/write)
0.040299  8
0.034032 16
0.029011 32
0.028218 64
0.029134 secs  2.14525 GiB/s 128

// 32 x 32 threadgrup with 256 elemetns to read by thread
0.01556  4 GiB
```
