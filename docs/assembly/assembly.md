# Specification for the assembly language

## Syntax

#### Keywords
- r0 through r9: reserved for registry names

#### Literals
- Numbers are assumed base ten, unless they have on of these prefixes:
    - `0b`: binary
    - `0`: octal
    - `0x`: hex
- Numbers may contain underscores anywhere but the first digit (nor immediately following the prefix).
- Binary and hex numbers must have at least one digit after the prefix.
- Character literals must be inbetween single quotes ('a').
- String literals must be inbetween double quotes ("a").

#### Identifiers
- Identifiers may NOT shadow keywords
- Identifiers are allowed to contain any alpha-numerical characters, plus any of the following `-_.:`.
- Identifiers must NOT start with a number

#### Tags
- Tags are defined with a trailing colon (:), which will not be part of the identifier.

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