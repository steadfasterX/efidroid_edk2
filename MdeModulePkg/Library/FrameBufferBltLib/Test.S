    .text
 .arm
    .syntax unified

 .fpu neon
 .arch armv7-a

 .macro def_fn f p2align=0
 .text
 .p2align \p2align
 .global \f
 .type \f, %function
\f:
 .endm

def_fn FrameBufferBltLibCopy32BitTo24Bit p2align=6
    // check if n>=7
    cmp     r2, #7
    movle   r0, #0
    bxle    lr

    mov     r3, r2

    // copy 8 pixels per iteration
loop1:
    vld4.8  {d0-d3}, [r1]
    vst3.8  {d0-d2}, [r0]
    add     r0, r0, #(8*3)
    add     r1, r1, #(8*4)
    sub     r3, r3, #8
    cmp     r3, #7
    bgt     loop1

    // return n%8
    sub     r0, r2, r3
    bx      lr

def_fn FrameBufferBltLibCopy24BitTo32Bit p2align=6
    // check if n>=7
    cmp     r2, #7
    movle   r0, #0
    bxle    lr

    mov     r3, r2

    // copy 8 pixels per iteration
loop2:
    vld3.8  {d0-d2}, [r1]
    vst4.8  {d0-d3}, [r0]
    add     r0, r0, #(8*4)
    add     r1, r1, #(8*3)
    sub     r3, r3, #8
    cmp     r3, #7
    bgt     loop2

    // return n%8
    sub     r0, r2, r3
    bx      lr
