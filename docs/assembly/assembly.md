# Specification for the assembly language

## Syntax

#### Keywords
- `r0` through `r9`: reserved as registry names
- The opcodes of the instruction set (see [spec.txt](../spec/spec.txt#L35))

#### Literals
- Numbers are assumed base ten, unless they have on of these prefixes:
    - `0b`: binary
    - `0`: octal
    - `0x`: hex
- Numbers may contain underscores anywhere but the first digit (nor immediately following the prefix).
- Binary and hex numbers must have at least one digit after the prefix.
- Character literals must be inbetween single quotes (`'a'`).
- String literals must be inbetween double quotes (`"Hello, world"`).

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
    add r0 r0 0x1
    eq r0 'z'
    out r0
    jt loop
```
Tokenization:
```
<VERB push> <REGISTER 0> <CHARACTER a> <EOL>
<TAG_DECL loop> <EOL>
<VERB add> <REGISTER 0> <REGISTER 0> <NUMBER 1> <EOL>
<VERB eq> <REGISTER 0> <CHARACTER z> <EOL>
<VERB out> <REGISTER 0> <EOL>
<VERB jt> <TAG_REF loop> <EOL>
<END>
```

## Grammar


| **Non-terminal** | **Explanation**              |
| ---------------- | ---------------------------- |
| `S`              | Start                        |
| `$`              | End                          |
| `E`              | Line                         |
| `I`              | Instruction                  |
| `T`              | Tag declaration              |
| `W`              | Word                         |
| `D`              | Raw data                     |
| `3`              | `VERB` expecting 3 arguments |
| `2`              | `VERB` expecting 2 arguments |
| `1`              | `VERB` expecting 1 arguments |
| `0`              | `VERB` expecting 0 arguments |

| **Terminal** | Matching token      | Explanation                    |
| ------------ | ------------------- | ------------------------------ |
| `x`          | `NUMBER_LITERAL`    |
| `c`          | `CHARACTER_LITERAL` |
| `s`          | `STRING_LITERAL`    |
| `r`          | `REGISTER`          |
| `t`          | `TAG_DECL`          | declaration of a tag           |
| `a`          | `TAG_REF`           | referencing a tag (a: address) |
| `'jmp'`      | `VERB`              | verb with the specified name   |
| `n`          | `EOL`               | end of line                    |

### Production rules

```
S → E$

# Three types of line
E → Tn | In | Dn

# Line with a tag declaration
T → t

# Line with an instruction
I → 3WWW | 2WW | 1W | 0

# Line with raw data
D → WD | sD | ε

# Single-word literals (plus references and registers)
W → x | c | a | r

# Instruction set (according to number of arguments)
0 → 'halt' | 'ret' | 'noop'
1 → 'push' | 'pop' | 'jmp' | 'call' | 'out'  | 'in'
2 → 'set'  | 'jt'  | 'jf'  | 'not'  | 'rmem' | 'wmem'
3 → 'eq'   | 'gt'  | 'add' | 'mult' | 'mod'  | 'and'  | 'or'
```

## FIRST/FOLLOW table
Let's first define a few sets for easier readability.
```
v0 = { 'halt', 'ret', 'noop' }
v1 = { 'push', 'pop', 'jmp', 'call', 'out' , 'in' }
v2 = { 'set' , 'jt' , 'jf' , 'not' , 'rmem', 'wmem' }
v3 = { 'eq'  , 'gt' , 'add', 'mult', 'mod',  'and', 'or' }
ω = v3 ∪ v2 ∪ v1 ∪ v0
```
Here is the resulting table:

| **Non-terminal** | **FIRST()**                 | **FOLLOW()**         |
| ---------------- | --------------------------- | -------------------- |
| S                | `{n, t, c, a, r, s, ε} ∪ ω` |                      |
| E                | `{n, t, c, a, r, s, ε} ∪ ω` | `{$}`                |
| T                | `{n, t}`                    | `{n}`                |
| I                | `ω`                         | `{n}`                |
| D                | `{x, c, a, r, s, ε}`        | `{n}`                |
| W                | `{x, c, a, r}`              | `{x, c, a, r, s, n}` |
| 0                | `v0`                        | `{n}`                |
| 1                | `v1`                        | `{x, c, a, r}`       |
| 2                | `v2`                        | `{x, c, a, r}`       |
| 3                | `v3`                        | `{x, c, a, r}`       |


### LL(1) table

|     | `x`    | `c`    | `s`    | `r`    | `t`    | `a`    | `v0`    | `v1`    | `v2`    | `v3`     | `n`    | `$`    |
| --- | ------ | ------ | ------ | ------ | ------ | ------ | ------- | ------- | ------- | -------- | ------ | ------ |
| `S` | `S→E$` | `S→E$` | `S→E$` | `S→E$` | `S→E$` | `S→E$` | `S→E$`  | `S→E$`  | `S→E$`  | `S→E$`   | `S→E$` | `S→E$` |
| `S` | `E→Dn` | `E→Dn` | `E→Dn` | `E→Dn` | `E→Tn` | `E→Dn` | `E→I`   | `E→I`   | `E→I`   | `E→I`    | `E→Sn` | `E→$`  |
| `T` |        |        |        |        | `T→t`  |        |         |         |         |          |        |        |
| `I` |        |        |        |        |        |        | `I→0`   | `I→1W`  | `I→2WW` | `I→3WWW` |        |        |
| `D` | `D→xD` | `D→WD` | `D→sD` | `D→WD` |        | `D→WD` |         |         |         |          | `D→ε`  |        |
| `W` | `W→x`  | `W→c`  |        | `W→r`  |        | `W→a`  |         |         |         |          |        |        |
| `0` |        |        |        |        |        |        | `0->v0` |         |         |          |        |        |
| `1` |        |        |        |        |        |        |         | `1->v1` |         |          |        |        |
| `2` |        |        |        |        |        |        |         |         | `2->v2` |          |        |        |
| `3` |        |        |        |        |        |        |         |         |         | `3->v3`  |        |        |
