set r0 1
jt r0 code2

code1:
    out '1'
    halt

code2:
    out '2'

set r0 0
jt r0 code1
