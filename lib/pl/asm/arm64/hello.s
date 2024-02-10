.global _start
.align 2

.data
msg: .ascii        "hello, arm64!\n"

.text
_start:
    mov     x0,  #1    // stdout
    ldr     x1,  =msg  //
    mov     x2,  14    // len(msg)
    mov     w8,  #64   // write syscall num
    svc     #0         // invoke syscall

    mov     x0,  #0    // exit status
    mov     x8,  #93   // exit syscall num
    svc     #0         // invoke syscall

