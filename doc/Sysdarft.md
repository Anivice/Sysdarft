# ![](sysdarft.png)

# **Content**

* 1. CPU Registers
  + i. General-Purpose Registers
    - Fully Extended Registers
    - Half-Extended Registers
    - Extended Registers
    - Registers
  + ii. Special-Purpose Registers
    - Segmented Addressing
      - Program Relocation
      - Segmentation
    - Stack Management
      - Definition of Stack
      - Stack Base Register
      - Stack Pointer
      - Stack Overflow
    - Code Segment
    - Data Segment

* 2. Assembler Syntax
  - Preprocessor directives
    - .org
      - Syntax
      - Explanation
      - Usage Example
    - .equ
      - Syntax
      - Explanation
      - Usage Example
    - .lab (*deprecated*)
      - Syntax
      - Explanation
      - Usage Example
  - Instructions
    - Operation Width
    - Operands
      - Registers
      - Constants
      - Memory References
      - Line Markers

3. Interruption
4. Instructions Set
5. Byte Code Format

# **CPU Registers**

*Registers are primitives used in hardware design that
are also visible to the programmer when the computer is completed,
so you can think of registers as the bricks of computer construction.*
[@ComputerOrganizationAndDesign]

Registers are generally preferred compared to accessing memory directly,
since the average time of reading/writing to registers is much lower than that
of the memory.

For Sysdarft, there are two register types:
**General-Purpose Registers** and **Special-Purpose Registers**.

## General-Purpose Registers

There exist sixteen general-purpose registers.
Each of the sixteen general-purpose registers is $64\text{-bit}$ in width,
meaning each has $64$ binary bits to store data.

### Fully Extended Registers

Fully extended registers (FERs) are the sixteen $64\text{-bit}$ general-purpose registers
mentioned above, named `%FER0`, `%FER1`, ..., `%FER15`.

### Half-Extended Registers

Half-extended registers, or HERs, are $32\text{-bit}$ registers for general purposes.
There are eight HERs, named `HER0`, `HER1`, ..., `HER7`.

The eight $32\text{-bit}$ registers are actually derived from the first four $64\text{-bit}$ registers,
specifically `FER0` through `FER3`.
This means that modifying the contents of *either* of the $32\text{-bit}$ or $64\text{-bit}$
versions of these registers will affect content of *both* of the $32\text{-bit}$ and $64\text{-bit}$ registers,
as they share the same underlying space.

### Extended Registers

Extended registers[^EXR], or EXRs, are $16\text{-bit}$ registers used for general purposes.
There are 8 EXRs, named `EXR0`, `EXR1`, ..., `EXR7`.
Similar to *Half-Extended Registers*,
these $16\text{-bit}$ registers are derived from the first four $32\text{-bit}$ registers,
specifically `HER0` through `HER3`.

[^EXR]:
The reason why $16\text{-bit}$ registers are called *Extended Registers*
is that they extend the original $8\text{-bit}$ registers.
In contrast, $32\text{-bit}$ registers are referred to as *Half-Extended Registers*
is because they are half the size of *Fully Extended Registers*, which are $64\text{-bit}$ in width.

Similarly,
this means that modifying the contents of either type of registers will affect content inside
all types of registers.

### Registers

Registers are $8\text{-bit}$ CPU registers used for general purposes.
There are eight Registers, named `R0`, `R1`, ..., `R7`.
These $8\text{-bit}$ registers are derived from the first four $16\text{-bit}$ registers,
specifically `EXR0` through `EXR3`.
And modifying *any* of the $8\text{-bit}$ registers will affect *all* types of registers sharing
the same space.

*The reason behind designing different width types of registers sharing the same area is that
it enables splitting data into width-specific pieces through register related operations alone
without relying on complicated bitwise operations or accessing external memory space.*

## Special-Purpose Registers

### Segmented Addressing

*A chunk of memory is known as a segment and hence the phrase
'segmented memory architecture.'
...,
A memory location is identified with a segment and an offset address
and the standard notation is `segment:offset`.*[@SoftwareDevelopmentForEngineers]

*In segmentation, an address consists of two parts: a segment number and a segment offset.
The segment number is mapped to a physical address,
and the offset is added to find the actual physical address.*[@ComputerOrganizationAndDesign]

Segmentation and segmented addressing were firstly used back in the 1970s,
in Intel 8086, the first usable processor at home to avoid installing an electronic giant.
Segmented addressing was used to extend its $16\text{-bit}$ memory bus to $20\text{-bit}$ in width,
with its internal registers remaining at $16\text{-bit}$.
This means the actual physical address is calculated by the following formula:

$\text{Physical Address} = \text{Segment Address} \ll 4 + \text{Segment Offset}$ 

*where*

- *Physical Address* being the actual location in the computer’s physical memory (Random Access Memory)
- *Segment Address* being the address of the segment, which is its physical location shifting four bits
                    to the right.
- *Segment Offset* being the length from the current position to the start of the segment.
- *$\ll$* being the bit left shifting operation, shifting bits towards the left.

In Sysdarft, it uses a $64\text{-bit}$ wide memory bus.


#### Program Relocation

As a user program, its memory location is arbitrary and should not be predetermined.
The operating system loads it wherever it deems appropriate.
While the start address of the program in memory is unknown,
its internal structure is known to the programmer.
The operating system loads the entire program as continuous data,
which means the structure within the program remains consistent.

Therefore, two parameters are used to determine the exact memory location of an arbitrary point in a user program:

- **The start of the program**: Managed by the operating system and unknown to the programmer.
- **The length from the current point to the start of the program**: Known and manageable by the programmer.

#### Segmentation

Segment, or segmentation, refers to a continuous block of data within a memory region. Each segment has:

- A **Base Address**: the start of the segment.
- A **Pointer** or **Offset**: the distance from the current location to the start point.

The exact memory location of the current point can be obtained by:

$\text{Absolute Address} = \text{Base Address} + \text{Offset}$

**where**

**Absolute Address** being the actual, non-segmented address in memory.

### Stack Management

#### Definition of Stack

Stack is mainly used for storing function return addresses in control flow management, local variables,
temporary data storage and CPU state protection.

Stack operates on a Last-In-First-Out basis,
meaning the last element pushed inside the stack will be popped at first, similar to a gun magazine.

#### Stack Base Register

Stack base, or `%SB`, is a $64\text{-bit}$ register that stores the start point of stack space.

#### Stack Pointer

Stack pointer, or `%SP`, is a $64\text{-bit}$ register that stores the **end** of the **usable** stack space.

The stack grows downwards, meaning data is stored from the end toward the start.
This design simplifies stack allocation:
by setting the pointer to a specific size, the stack is automatically sized accordingly.

The following is a demonstration illustrating how a stack is managed
(Push and Pop Data onto the Stack):

Suppose the stack pointer `%SP` initially points to address `0x1000` and `%SB` points to `0xFFFF`.

When a value is pushed onto the stack, the `%SP` is decreased (since the stack grows downward),
and the value is stored at the new address.

```c
   (Stack End)        (Stack Pointer)      (Stack Base)
        [ -- 8 Byte Data -- ][ -- Free Space -- ]
        ^                   ^                   ^
        |                   |                   |
  0xFFFF+0x1000       0xFFFF+0x0FF8       0xFFFF+0x0000
```

When a value is popped from the stack, the `%SP` is increased and the stack grows back up,
freeing the space in the process.

```c
   (Stack End)    (Stack Pointer After Pop)                      (Stack Base)
        [ -- 7 Byte Data -- ][ --- --- --- Free Space -- --- --- --- ]
                             [ -- 1 Byte Data -- ]
                                      (Stack Pointer Before Pop)
        [ --- --- ----  8 Byte Data ---- --- --- ][ -- Free Space -- ]
        ^                    ^                   ^                   ^
        |                    |                   |                   |
  0xFFFF+0x1000        0xFFFF+0x0FF9       0xFFFF+0x0FF8       0xFFFF+0x0000
```

#### Stack Overflow

There exists a situation where an operation attempts to store data that is larger than the available stack space.
This means `%SP` will be attempted to be set to a negative number.

And if `%SP` decreases below zero, the register will overflow.

An overflow occurs when an operation attempts to store data beyond the capacity of a register or memory region.
This can happen if a value is too large or becomes so small that it goes below zero.

In this context, when `%SP` goes below zero, the binary bits will "wrap around."
For example, when `%SP` becomes `-1`, it wraps around to `0xFFFFFFFFFFFFFFFF` (`18446744073709551615`),
where every bit in the register is set to `1`.
This means the register holds the maximum value a $64\text{-bit}$ register can represent.

As a result, the stack pointer will point to an address that a $64\text{-bit}$ system cannot access.
This occurs because the pointer, when combined with its base address,
will reference a location almost certainly beyond the $64\text{-bit}$ addressable space.

This situation is called a stack overflow:
an overflow of the stack pointer that leads to a damaged stack frame (lost stack location).

### Code Segment

The code segment is typically managed by the operating system rather than the user.
The pointer for this segment, the instruction pointer (`IP`), is inaccessible,
even to the operating system.
However, the Code Base (`%CB`) register is accessible and can be used to set up alternative code spaces.

Directly modifying `%CB` would cause the CPU to perform an unpredictable jump (a "wild jump").
Therefore, `%CB` is usually not modified directly but rather changed indirectly through operations
like a long call or long jump.

### Data Segment

There are four registers that can be used together to reference two data segments:
Data Base (`%DB`), Data Pointer (`%DP`), Extended Base (`%EB`), and Extended Pointer (`%EP`).
These registers function in pairs—`%DB` with `%DP` and `%EB` with `%EP`—to address and access
two data segments.

# **Assembler Syntax**

An assembler is a compiler that translates human-readable machine instructions into machine-readable binary.

Sysdarft assembler, like other assemblers, is case-insensitive.

## Preprocessor directives

There are three preprocessor directives for Sysdarft Assembly Language.

### .org

#### Syntax

```asm
    .org [Decimal or Hexadecimal]
```

#### Explanation

`.org`, or origin, defines the code line offset in the whole memory.
Normally origin starts from `0x00`, but some code, like BIOS, for example,
loads at `0xC1800`, thus all line markers will be incorrect when offset 
calculation starts from `0x00`.
Thus, using `.org` as a way to define the origin of the code.

#### Usage Example

```asm
    .org 0xC1800
```

### .equ

#### Syntax

```asm
    .equ '[Search Target]', '[Replacement]'
```

#### Explanation

In assembly or low-level programming, the `.equ` directive is used to
**replace occurrences of a string** with another, similar to how macros work in C.
It's essentially a way to define **symbolic constants** or **aliases** for values
or strings.

- **When compiler disable the regular expression support**

  If the **compiler or assembler doesn't enable regular expressions**
  for the `.equ` directive, it will simply perform a **literal string replacement**.
  In this case, it will search for occurrences of a specific string
  (**Search Target**) and replace them with the **Replacement**
  exactly as they appear, without any special pattern matching or modifications.

- **When compiler enable the regular expression support**

  If the **compiler or assembler enabled regular expressions**,
  the `.equ` directive can behave like a **regular expression search-and-replace**.
  This means it can **capture groups** (just like `sed -E`),
  and more complex patterns can be used in replacements.
  This allows for more flexible and powerful string matching.

#### Usage Example

```asm
    .equ 'HDD_IO', '0x1234'
    .equ 'add\s*\((.*), (.*)\)', 'add .32bit \1, \2'
```

### .lab (*deprecated*)

#### Syntax

```asm
    .lab marker1, [marker2, ...]
```

#### Explanation

Define one or more line markers.
This directive is deprecated.
Line markers can be auto scanned and defined without relying on this directive.

#### Usage Example

```asm
    .lab _start, _end
```

## Instruction Expression

For all instruction expressions, it follows this syntax:

```asm
    Mnemonic [Operation Width] <Operand1> [, <Operand2>]
```

Operation width is enforced by many data-modifying instructions.
It refers to the data width of one or two of the instruction's operands.
When two operands are provided, both must have the same data width.

The following is the breakdown of each part of the instruction expression.

### Mnemonic

Mnemonic is a symbolic name for a single executable machine language instruction.
Refer to the **Instruction Set** for the whole instruction mnemonic table.

### Operation Width

Operation width can be `.8bit`, `.16bit`, `.32bit`, or `.64bit`,
representing $8\text{-bit}$, $16\text{-bit}$, $32\text{-bit}$ and $64\text{-bit}$ data width respectively.

### Operands

Operands need to be enclosed within `<` and `>`.
There are three possible operand types: registers, constants, or memory references.

#### Registers

Register operands are internal CPU registers: general-purpose or special-purpose.

Registers must start with `%`, with no space between `%` and register name.
For example: `%EXR2` is valid, but `"% EXR2"` is not and will not be detected as an operand.

#### Constants

A constant is an expression consisting of one or more decimal and/or hexadecimal numbers.

The preprocessor first transforms hexadecimal values into decimals,
then runs the expression through the `bc` calculator.
Valid `bc` expressions are accepted as long as the output is a decimal.

Constant expressions are always enclosed by `$(` and `)`.
Expression, if being a stand-alone operand, is enclosed by `<` and `>`,
resulting in a double enclosure of both signs.
For example, a constant in an instruction expression can look like this:

```asm
    add .64bit <%FER0>, < $( 0xFFFF + 0xBC ) >
```

Constant expressions are always 64 bits wide.
Any value exceeding 64 bits will trigger an overflow report but is not considered an error.
In the event of an overflow, the result is set to `ULLONG_MAX` (`18446744073709551615`).

#### Memory References

Memory references are data stored at a specific memory location.

Memory references are a complicated expression:

```asm
    *[Ratio]&[Width](Base, Offset1, Offset2)
```

**and**

$\text{Memory Linear Address} = \text{Ratio} \times ( \text{Base} + \text{Offset1} + \text{Offset2} )$

**where**

- `Ratio` can be `1`, `2`, `4`, `8`, `16`.
- `Base`, `Offset1`, `Offset2` can be either constant expressions or registers.
- `Width` specifies data width of the memory location, which can be `8`, `16`, `32`, `64`,
  representing $8\text{-bit}$, $16\text{-bit}$, $32\text{-bit}$, and $64\text{-bit}$ data respectively.

The following is an example of a memory reference:

```asm
    *2&64(%FER1, $(0xFC), $(0xBC))
```

This address points to a $64\text{-bit}$ data width space at the address
$(\text{\%FER1} + \text{0xFC} + \text{0xBC}) \times 2$

#### Line Markers

Line markers are special operands that record the offset of their corresponding code.

For example:

```asm
    jmp <%cb>, <_start>

    _start:
        xor .32bit <%her0>, <%her0>
```

`_start` is identified as a line marker by a following `:`.
Only spaces and tabs may appear after the colon, any other elements like instructions will be considered as errors.

If `.org` is not specified, line markers are calculated as offsets from the beginning of the file, starting at `0`.
If `.org` is specified, the offset is calculated from the given address.

# **Interruption**

Interrupt, or interruption, 

# **Instructions Set**

## Miscellaneous

### **NOP**     

`NOP` means No Operation. Do nothing.

The opcode[^1] for `NOP` is `0x00`,
which is the default value when memory initialized
and the default value used for peddling[^2].

[^1]: opcode: The field that denotes the operation and format of an instruction [@ComputerOrganizationAndDesign].

[^2]: When a field following another field will not fit into a partially-filled storage unit,
it may be split between units, or the unit may be padded.
An unnamed field with width 0 forces this padding,
so that the next field will begin at the edge of the next allocation unit.
[@TheCProgrammingLanguage]

| **HLT**     | Halt the CPU, then shutdown     | `HLT`  |
| **IGNI**    | Set IM (Interruption Mask) to 1 | `IGNI` |
| **ALWI**    | Set IM (Interruption Mask) to 0 | `ALWI` |

## Arithmetic

| Instruction | Explanation                                                      | Syntax                       |
|-------------|------------------------------------------------------------------|------------------------------|
| **ADD**     | `OPR1` $=$ `OPR1` + `OPR2`                                       | `ADD [Width] <OPR1>, <OPR2>` |
| **ADC**     | `OPR1` $=$ `OPR1` + `OPR2` + `CF`                                | `ADC [Width] <OPR1>, <OPR2>` |
| **SUB**     | `OPR1` $=$ `OPR1` - `OPR2`                                       | `SUB [Width] <OPR1>, <OPR2>` |
| **SBB**     | `OPR1` $=$ `OPR1` - `OPR2` - `CF`                                | `SBB [Width] <OPR1>, <OPR2>` |
| **IMUL**    | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` $\times$ (signed) `OPR1`   | `IMUL [Width] <OPR1>`        |
| **MUL**     | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` $\times$ (unsigned) `OPR1` | `MUL [Width] <OPR1>`         |
| **IDIV**    | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` / (signed) `OPR1`          | `IDIV [Width] <OPR1>`        |
| **DIV**     | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` / (unsigned) `OPR1`        | `DIV [Width] <OPR1>`         |
| **NEG**     | `OPR1` $=$ $-$`OPR1`                                             | `NEG [Width] <OPR1>`         |
| **CMP**     | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |
| **INC**     | `OPR1` $=$ `OPR1` $+ 1$                                          | `INC [Width] <OPR1>`         |
| **DEC**     | `OPR1` $=$ `OPR1` $- 1$                                          | `DEC [Width] <OPR1>`         |

## Data Transfer

| Instruction | Explanation                                                      | Syntax                       |
|-------------|------------------------------------------------------------------|------------------------------|
| **MOV**     | `OPR1` $=$ `OPR1` + `OPR2`                                       | `ADD [Width] <OPR1>, <OPR2>` |
| **XCHG**    | `OPR1` $=$ `OPR1` + `OPR2` + `CF`                                | `ADC [Width] <OPR1>, <OPR2>` |
| **PUSH**    | `OPR1` $=$ `OPR1` - `OPR2`                                       | `SUB [Width] <OPR1>, <OPR2>` |
| **POP**     | `OPR1` $=$ `OPR1` - `OPR2` - `CF`                                | `SBB [Width] <OPR1>, <OPR2>` |
| **PUSHALL** | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` $\times$ (signed) `OPR1`   | `IMUL [Width] <OPR1>`        |
| **POPALL**  | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` $\times$ (unsigned) `OPR1` | `MUL [Width] <OPR1>`         |
| **ENTER**   | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` / (signed) `OPR1`          | `IDIV [Width] <OPR1>`        |
| **LEAVE**   | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` / (unsigned) `OPR1`        | `DIV [Width] <OPR1>`         |
| **MOVS**    | `OPR1` $=$ $-$`OPR1`                                             | `NEG [Width] <OPR1>`         |
| **LEA**     | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |

## Logic and Bitwise

| Instruction | Explanation                                                      | Syntax                       |
|-------------|------------------------------------------------------------------|------------------------------|
| **AND**     | `OPR1` $=$ `OPR1` + `OPR2`                                       | `ADD [Width] <OPR1>, <OPR2>` |
| **OR**      | `OPR1` $=$ `OPR1` + `OPR2` + `CF`                                | `ADC [Width] <OPR1>, <OPR2>` |
| **XOR**     | `OPR1` $=$ `OPR1` - `OPR2`                                       | `SUB [Width] <OPR1>, <OPR2>` |
| **NOT**     | `OPR1` $=$ `OPR1` - `OPR2` - `CF`                                | `SBB [Width] <OPR1>, <OPR2>` |
| **SHL**     | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` $\times$ (signed) `OPR1`   | `IMUL [Width] <OPR1>`        |
| **SHR**     | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` $\times$ (unsigned) `OPR1` | `MUL [Width] <OPR1>`         |
| **ROL**     | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` / (signed) `OPR1`          | `IDIV [Width] <OPR1>`        |
| **ROR**     | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` / (unsigned) `OPR1`        | `DIV [Width] <OPR1>`         |
| **RCL**     | `OPR1` $=$ $-$`OPR1`                                             | `NEG [Width] <OPR1>`         |
| **RCR**     | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |

## Control Flow

| Instruction | Explanation                                                      | Syntax                       |
|-------------|------------------------------------------------------------------|------------------------------|
| **JMP**     | `OPR1` $=$ `OPR1` + `OPR2`                                       | `ADD [Width] <OPR1>, <OPR2>` |
| **CALL**    | `OPR1` $=$ `OPR1` + `OPR2` + `CF`                                | `ADC [Width] <OPR1>, <OPR2>` |
| **RET**     | `OPR1` $=$ `OPR1` - `OPR2`                                       | `SUB [Width] <OPR1>, <OPR2>` |
| **JA**      | `OPR1` $=$ `OPR1` - `OPR2` - `CF`                                | `SBB [Width] <OPR1>, <OPR2>` |
| **JNE**     | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` $\times$ (signed) `OPR1`   | `IMUL [Width] <OPR1>`        |
| **JB**      | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` $\times$ (unsigned) `OPR1` | `MUL [Width] <OPR1>`         |
| **JL**      | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` / (signed) `OPR1`          | `IDIV [Width] <OPR1>`        |
| **JBE**     | `R/EXR/HER/FER0` $=$ `R/EXR/HER/FER0` / (unsigned) `OPR1`        | `DIV [Width] <OPR1>`         |
| **JLE**     | `OPR1` $=$ $-$`OPR1`                                             | `NEG [Width] <OPR1>`         |
| **JC**      | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |
| **JNC**     | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |
| **JO**      | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |
| **JNO**     | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |
| **LOOP**    | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |
| **INT**     | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |
| **INT3**    | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |
| **IRET**    | Compare `OPR1` with `OPR2`, and set corresponding flags          | `CMP [Width] <OPR1>, <OPR2>` |

## Input/Output

| Instruction | Explanation                                                      | Syntax                       |
|-------------|------------------------------------------------------------------|------------------------------|
| **IN**      | `OPR1` $=$ `OPR1` + `OPR2`                                       | `ADD [Width] <OPR1>, <OPR2>` |
| **OUT**     | `OPR1` $=$ `OPR1` + `OPR2` + `CF`                                | `ADC [Width] <OPR1>, <OPR2>` |
| **INS**     | `OPR1` $=$ `OPR1` - `OPR2`                                       | `SUB [Width] <OPR1>, <OPR2>` |
| **OUTS**    | `OPR1` $=$ `OPR1` - `OPR2` - `CF`                                | `SBB [Width] <OPR1>, <OPR2>` |

# References
