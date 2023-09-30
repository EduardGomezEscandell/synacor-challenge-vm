loop:
    in r0
    out r0
    eq r7 r0 '!'
    jf r7 loop

out '\n'