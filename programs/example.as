set r0 'a'
loop:
    add r0 r0 1
    eq r1 r0 'z'
    out r0
    jt r1 loop
