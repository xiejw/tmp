.text
.global result_

// objdump -d bin/ch02_01_.o

result_:
  add r0, r0, r1
  sub r0, r0, r2
  bx lr
