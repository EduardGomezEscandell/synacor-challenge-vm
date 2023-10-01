# Synacor challenge VM and assembler

This repository contains a work-in-progress version of a virtual machine following the specifications of the Synacor challenge.
Unfortunatelly, it seems that the website of the challenge is down, but you can read the spec in [here](./docs/spec/spec.txt).

This repository also implements my assembly language with an assembler for the Synacor architecture. You can read the specification of the assembly language [here](./docs/assembly/assembly.md).

## Build the project
To build the project you need `cmake` and a compiler that supports C++20. To compile, simply do:
```bash
./make
```

The executables will land in the `build` folder. If you want more control over the build, use any of these environment varibles:
- `BUILD_TYPE`: Either `Release` or `Debug`. Default: `Release`.
- `ENABLE_SANITIZER`: Either `1` or empty. Default: empty.
- `BUILD_TESTS`: Either `1` or empty. Default: empty.

## Assemble a program
In order to assemble, for example [hello-world.as](./example-programs/hello-world.as):
```bash
./build/Release/assembler/cmd/assemble  \
    example-programs/hello-world.as     \
    hello-world.syn
```

## Run a binary
After having assembled a binary, you can run it with:
```bash
./build/Release/vm/cmd/runvm hello-world.syn
```
