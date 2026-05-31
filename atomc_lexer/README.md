# AtomC Lexical Analyzer

Implementation of the homework from **Laboratory 3 (Compilation Techniques)** –
Politehnica University Timișoara.

## What it does

Reads an AtomC source file and produces the list of lexical tokens
(token code + attribute when applicable), exactly as required by the
"Lexical Analyzer (Deadline W6)" assignment.

## How it maps to the FSM diagram (FSM_Diagram-2.pdf)

`getNextToken()` uses **explicit states** whose numbers correspond to the
states drawn in the FSM diagram:

| State | Purpose (matches the drawing)                                    |
|:-----:|------------------------------------------------------------------|
|   0   | initial / dispatch (the centre of the diagram)                   |
|   1   | consumed `'0'` (could be 0, octal, hex, or `0.xxx`)               |
|   2   | consumed `[1-9]`, collecting decimal digits                      |
|   4   | consumed `0x`, expecting first hex digit                         |
|   5   | collecting hex digits → **CT_INT**                               |
|   7   | consumed `'.'`, expecting first fractional digit                 |
|   8   | collecting fractional digits                                     |
|   9   | consumed `e`/`E`, expecting sign or digit                        |
|  10   | consumed sign after `e`, expecting digit                         |
|  11   | collecting exponent digits → **CT_REAL**                         |
|  19   | inside `'…'` → **CT_CHAR** (states 20, 21 folded inline)         |
|  22   | inside `"…"` → **CT_STRING** (closes on a non-escaped `"`)       |
|  24   | collecting identifier chars; on else → keyword check or **ID**   |

Single-character delimiters and operators branch from **state 0**
(which plays the role of hub-state "26" in the diagram); two-char
operators (`&&`, `||`, `==`, `!=`, `<=`, `>=`) read the second char
inline before deciding which token to emit. **SPACE** and **LINECOMMENT**
just consume input and return to state 0 (no token is generated), exactly
as in the diagram.

## Build (VS Code)

Open the folder in Visual Studio Code and either:

* **Ctrl + Shift + B** – runs the *Build Lexer* task
* Or in a terminal:

```bash
gcc -Wall -Wextra -std=c99 -o lexer lexer.c
```

## Run

```bash
./lexer tests/0.c        # any of tests/0.c … tests/9.c
```

To run **all tests** at once:

```bash
make test
```

To run a single test by number:

```bash
make run T=8             # runs lexer on tests/8.c
```

## Output format

```
LINE   ID   TOKEN       ATTRIBUTE
--------------------------------------------
2      15   VOID
2      5    ID          "main"
2      19   LPAR
2      20   RPAR
3      23   LACC
4      5    ID          "put_s"
4      19   LPAR
4      4    CT_STRING   "hello"
4      20   RPAR
4      18   SEMICOLON
5      24   RACC
7      0    END
```

For every token the analyser prints:
* the source line
* the numeric code
* the symbolic name
* the attribute (only for **ID**, **CT_STRING**, **CT_INT**, **CT_CHAR**, **CT_REAL**)

## What is recognised

| Category    | Tokens                                                                |
|-------------|-----------------------------------------------------------------------|
| Literals    | `CT_INT` (dec / octal / hex), `CT_REAL`, `CT_CHAR`, `CT_STRING`       |
| Identifier  | `ID` (`[a-zA-Z_][a-zA-Z0-9_]*`)                                       |
| Keywords    | `break char double else for if int return struct void while`          |
| Delimiters  | `, ; ( ) [ ] { }` and `END` (`'\0'` / EOF)                            |
| Operators   | `+ - * / . && \|\| ! = == != < <= > >=`                               |
| Skipped     | whitespace, `\n`, `//` line comments                                  |

Escape sequences (`\n \t \r \\ \' \" \0`) are supported inside character
constants and string constants — needed to pass test `8.c`.
