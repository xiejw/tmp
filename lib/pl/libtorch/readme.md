Compilation

To compile with local pytorch
```
make [CPU=1]
```

To compile with libtorch
```
# macOS
# - brew install libomp
# Linux
# - n/a
make LIBTORCH=1 [CPU=1]
```
