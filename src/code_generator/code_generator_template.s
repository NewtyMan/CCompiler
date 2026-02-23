.text
.global _start

_start:
    bl main
    mov r7, #1
    svc 0

syscall_three_arg:
    str fp, [sp, #0]
    str lr, [sp, #4]
    mov fp, sp

    ldr r0, [fp, #12]
    ldr r1, [fp, #16]
    ldr r2, [fp, #20]
    svc 0

    ldr lr, [fp, #4]
    ldr fp, [fp, #0]
    add sp, sp, #24
    bx lr

close:
    str fp, [sp, #0]
    str lr, [sp, #4]
    mov fp, sp

    ldr r0, [fp, #12]
    mov r7, #6
    svc 0

    ldr lr, [fp, #4]
    ldr fp, [fp, #0]
    add sp, sp, #16
    bx lr

getrandom:
    mov r7, #384
    b syscall_three_arg

open:
    mov r7, #5
    b syscall_three_arg

read:
    mov r7, #3
    b syscall_three_arg

write:
    mov r7, #4
    b syscall_three_arg
