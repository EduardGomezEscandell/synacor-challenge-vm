# Synacor challenge VM and assembler

This repository contains an implementation of a virtual machine following the specifications of the Synacor challenge.
Unfortunatelly, it seems that the website of the challenge is down, but you can read the spec in [here](./docs/spec/spec.txt).

As a side quest, I decided to write a LL(1) parser for my made-up assembly language. You can read the specification of the assembly language [here](./docs/assembly/assembly.md).

## Build the project
To build the project you need `cmake` and a compiler that supports C++20. To compile, simply do:
```bash
./make
```

The executables will land in the `build` folder. If you want more control over the build, use any of these environment varibles:
- `BUILD_TYPE`: Either `Release` or `Debug`. Default: `Release`.
- `ENABLE_SANITIZER`: Either `1` or empty. Default: empty.
- `BUILD_TESTS`: Either `1` or empty. Default: empty.

## Solve the challenge
If you want to run the challenge (or any other synacor-compatible binary), you can do it with:
```bash
./build/Release/vm/cmd/runvm ./docs/spec/challenge
```

However, solving the challenge requires messing with the VM's registers. You can use the debugger-enabled VM via:
```bash
./build/Release/vm/cmd/vmctl ./docs/spec/challenge
```

Command `!help` shows all debug commands:
```
Use any of these commads.
You can escape the leading ! by writing !!
---------------------+-----------------------------------------------------
Usage                | Help
---------------------+-----------------------------------------------------
!abreak <ADDRESS>    | Toggles a breakpoint at the specified address.
!cont                | Continues execution (may not appear so if input is needed).
!cov                 | Toggle coverage profiling
!dump                | Dumps the current state of the memory to file heap.bin
!exit                | Stops the machine by overwriting a HALT at the current position pointed by the instruction pointer
!help                | Prints this message
!ibreak <INSTR>      | Toggles a breakpoint at the specified instruction
!instr               | Toggles instruction logging
!peek                | Shows the next instruction to execute. It also displays the registers
!rmem <ADDR>         | reads out the line of memory ADDR is in
!setr <REG> <VALUE>  | sets register REG to VALUE
!skip <N>            | Advances N instructions and then stops. It may stop earlier if STDIN input is needed, but it'll stop again in the specified point.
!step                | Advances one instruction. Equivalent to 'skip 1'
!wmem <ADDR> <VALUE> | writes value VALUE into memeory address ADDR.
---------------------+-----------------------------------------------------
```

If all you want is to see the challenge be solved in front of you, run `validate-challenge.sh`.

## Using the assembler
In order to assemble, for example [hello-world.as](./example-programs/hello-world.as):
```bash
./build/Release/assembler/cmd/assemble  \
    example-programs/hello-world.as     \
    hello-world.syn
```
