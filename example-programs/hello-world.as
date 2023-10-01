jmp main

data:
    "Hello, world!\0"

main:
    set r7 data
    
    loop:
        rmem r1 r7
        eq r0 r1 '\0'
        jt r0 endloop

        out r1

        add r7 r7 1
        jmp loop
    endloop:
    
    out '\n'
    halt
