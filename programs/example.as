push r0 'a'
loop:
    add r0 r1 0x
    eq r0 'z'
    out r0
    jt loop