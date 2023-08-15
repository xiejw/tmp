import torch
import time

print(f"torch mps backend: {torch.backends.mps.is_available()}")

# choose a device
device = "cpu"
#device = "mps"

# shapes must be consistent in all tests.
a = torch.randn([4096 * 2, 4096 * 2], device=device)
b = torch.randn([4096 * 2, 4096 * 2], device=device)

def sync():
    if device == "mps":
        torch._C._mps_synchronize()

sync()

# warm up
for i in range(10):
    c = torch.matmul(a, b)

# real game
count = 10
sync()
start = time.time()
for i in range(count):
    c = torch.matmul(a, b)
sync()
end = time.time()

print(f"c.device is {c.device}")
print(f"c.dtype is {c.dtype}")
print(f"c is {c}")
print(f"c.matmul takes {(end-start) / count }")
