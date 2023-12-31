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


| **Non-terminal** | **Explanation** |
| ---------------- | --------------- |
| `S`              | Start           |
| `$`              | End             |
| `P`              | Program         |
| `I`              | Instruction     |
| `T`              | Tag declaration |
| `W`              | Word            |
| `D`              | Raw data        |
| `R`              | A register      |

| **Terminal** | Matching token      | Explanation                  |
| ------------ | ------------------- | ---------------------------- |
| `x`          | `NUMBER_LITERAL`    |
| `c`          | `CHARACTER_LITERAL` |
| `s`          | `STRING_LITERAL`    |
| `r`          | `REGISTER`          |
| `t`          | `TAG_DECL`          | declaration of a tag         |
| `a`          | `TAG_REF`           | referencing a tag            |
| `'jmp'`      | `VERB`              | verb with the specified name |
| `n`          | `EOL`               | end of line                  |


### Production rules

```
S → P$

# Every line may be trailed by another line
# Four types of line (empty, declaration, instruction, data)
P → nP | TnP | InP | DnP | ε

# Line with a tag declaration
T → t

# Line with an instruction
I → 'halt'
I → 'set'RW
I → 'push'W
I → 'pop'R
I → 'eq'RWW
I → 'gt'RWW
I → 'jmp'W
I → 'jt'WW
I → 'jf'WW
I → 'add'RWW
I → 'mult'RWW
I → 'mod'RWW
I → 'and'RWW
I → 'or'RWW
I → 'not'RW
I → 'rmem'RW
I → 'wmem'WW
I → 'call'W
I → 'ret'
I → 'out'W
I → 'in'R
I → 'noop'

# Line with raw data
D → xD | cD | aD | rD | sD | ε

# Single-word literals (plus references and registers)
W → x | c | r
```
### A caviat
Note that this grammar is not truly LL(1): there is ambiguity in the presence of an emtpy line. The following program:
```

"this string literal sits under an empty line"
```
Note that the tokenizer always appends a newline at the end of non-empty files. Hence the previous text is tokenized into the following string:
```
nsn
```
Which can be parsed as any of these two trees:
```
              S                S
              |                |
              P                P
            / | \            / | \
           n  D  n          D  D  n
              |             |  |
              s             n  s
```
We could get rid of the ambiguity by removing rule `P → nP`, however it makes more sense to consider empty lines as their own thing than to lump them in with raw-data lines.

Hence, we default to the left tree.


## FIRST/FOLLOW table
The set `ω` represents any verb. Here is the resulting table:

| **Non-terminal** | **FIRST()**                    | **FOLLOW()**                |
| ---------------- | ------------------------------ | --------------------------- |
| S                | `{n, t, x, c, a, r, s, ε} ∪ ω` | `{$}`                       |
| P                | `{n, t, x, c, a, r, s, ε} ∪ ω` | `{$, n, t, c, a, r, s} ∪ ω` |
| T                | `{t}`                          | `{n}`                       |
| I                | `ω`                            | `{n}`                       |
| D                | `{x, c, a, r, s, ε}`           | `{x, c, a, r, s, n}`        |
| W                | `{x, c, a, r}`                 | `{x, c, a, r, s, n}`        |
| R                | `{r}`                          | `{x, c, a, r, s, n}`        |


### LL(1) table
The set `ω` represents any verb. To avoid spamming the table, the rules for each verb are skipped.

|     | `x`     | `c`     | `s`     | `r`     | `t`     | `a`     | `ω`      | `n`    | `$`   |
| --- | ------- | ------- | ------- | ------- | ------- | ------- | -------- | ------ | ----- |
| `S` | `S→P$`  | `S→P$`  | `S→P$`  | `S→P$`  | `S→P$`  | `S→P$`  | `S→P$`   | `S→P$` |       |
| `P` | `P→DnP` | `P→DnP` | `P→DnP` | `P→DnP` | `P→TnP` | `P→DnP` | `P→InP`  | `P→nP` | `P→$` |
| `T` |         |         |         |         | `T→t`   |         |          |        |       |
| `I` |         |         |         |         |         |         | `I→ω...` |        |       |
| `D` | `D→xD`  | `D→cD`  | `D→sD`  | `D→rD`  |         | `D→aD`  |          | `D→ε`  |       |
| `W` | `W→x`   | `W→c`   |         | `W→r`   |         | `W→a`   |          |        |       |
| `R` |         |         |         | `W→r`   |         |         |          |        |       |
