.global _start
.align 2

_start:
    mov     x0,  1
    adr     x1,  msg   // ldr does not work with apple silicon
    mov     x2,  14    // len(msg)
    mov     x16, 4     // write syscall num
    svc     128        // invoke syscall

    mov     x0,  0     // exit status
    mov     x16, 1     // exit syscall num
    svc     128        // invoke syscall

msg: .ascii        "hello, arm64!\n"
