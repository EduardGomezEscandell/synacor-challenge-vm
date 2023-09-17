# Specification for the assembly language

## Syntax

#### Keywords
- r0 through r9: reserved for registry names

#### Literals
- Numbers are assumed base ten, unless:
    - Preceded by 0b: binary
    - Preceded by 0: octal
    - Preceded by 0x: hex
- Numbers must have at least one digit (prefix not counted)
- Character literals are inbetween single quotes ('a').
- String literals are inbetween double quotes ("a").

#### Identifiers
- Identifiers may NOT shadow keywords
- Identifiers are allowed to contain any characters except spaces and newlines.
- Identifiers must NOT start with a number, single, or double quotes

#### Tags
- Tags are defined with a triling colon (:), which will not be part of the identifier.

### Example tokenization
```
push r0 'a'
loop:
    add r0 r1 0x1
    eq r0 'z'
    out r0
    jt loop
```
Tokenization:
```
<IDENTIFIER push> <REGISTER 0> <CHARACTER a> <EOL> 
<TAG_DECL loop> <EOL> 
<IDENTIFIER add> <REGISTER 0> <REGISTER 1> <NUMBER 1> <EOL> 
<IDENTIFIER eq> <REGISTER 0> <CHARACTER z> <EOL> 
<IDENTIFIER out> <REGISTER 0> <EOL> 
<IDENTIFIER jt> <IDENTIFIER loop> <EOL> 
<END>
```