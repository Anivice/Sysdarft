<style>
/* Page settings for page breaks, margins, and overall layout */
@page {
    size: A4;
    margin: 1in;
}

/* Apply only to the h1 element */
h1 {
    width: 100%; /* Ensure it takes full width */
    white-space: nowrap; /* Prevent line breaks */
    overflow: hidden;
    text-align: center; /* Center the text horizontally */
    display: flex;
    justify-content: center;
    align-items: center;
    font-family: Arial, sans-serif;
    overflow: hidden;
}

body {
    font-family: Arial, sans-serif;
    line-height: 1.5;
    margin-top: 1in;
}

/* Keep headers at the top of the page and prevent breaking across pages */
h2 {
    page-break-before: always;
    font-size: 18px;
}

/* Table-specific handling */
table {
    width: 100%;
    margin-left: auto;
    margin-right: auto;
    border-collapse: collapse;
    table-layout: fixed; 
}

th, td {
    padding: 8px;
    text-align: center;
    border: 1px solid #ddd;
    word-wrap: break-word;
  }

/* Prevent tables from being split across pages */
table {
    page-break-inside: avoid;
}

/* Keep header rows at the top of each page if the table spans multiple pages */
thead {
    display: table-header-group;
}

tfoot {
    display: table-footer-group;
}

/* Prevent section titles from appearing at the bottom of pages */
h1, h2, h3, h4, h5, h6 {
    page-break-after: avoid;
}

/* Add spacing between sections */
section {
    margin-bottom: 2em;
}
</style>

# Sysdarft Assembler and Emulator Documentation

## **Content**

1. [CPU Registers](#cpu-registers)
2. [Assembler Syntax](#assembler-syntax)
3. Instructions Set
4. Assembler and Emulator Usage
5. Debugger Backend API Documentation

## **CPU Registers**

### General Purpose Registers

#### Fully Extended Registers

Fully extended registers, or FERs, are 64-bit registers for general purposes.
There are 16 FERs, named `FER0`, `FER1`, ..., `FER15`.

#### Half-Extended Registers

Half-extended registers, or HERs, are 32bit registers for general purposes.
There are 8 HERs, named `HER0`, `HER1`, ..., `HER7`.
Here's a clearer and more polished version of the paragraph:

The eight 32-bit registers are actually derived from the first four 64-bit registers,
specifically `FER0` through `FER3`.
This means that modifying the contents of **either** the 32-bit or 64-bit versions of these registers
will also affect **both** the 32-bit and 64-bit registers,
as they share the same underlying memory space.

#### Extended Registers

Extended registers, or EXRs, are 16-bit registers used for general purposes.
There are 8 EXRs, named `EXR0`, `EXR1`, ..., `EXR7`.
These 16-bit registers are derived from the first four 32-bit registers,
specifically `HER0` through `HER3`.

This means that modifying the contents of **either** the 16-bit (EXR) or 32-bit (HER) versions
of these registers will affect **both** the 16-bit and 32-bit registers,
as they share the same underlying memory space.

#### Registers

Registers are 8-bit CPU registers used for general purposes.
There are eight Registers, named `R0`, `R1`, ..., `R7`.
These 8-bit registers are derived from the first four 16-bit registers,
specifically `EXR0` through `EXR3`.

This means that modifying the contents of **either** the 8-bit or 16-bit versions
of these registers will affect **both** the 8-bit and 16-bit registers,
as they share the same underlying memory space.

Below is a regester layout table for first four fully extended registers:

<table>
  <!-- FERs (64-bit) Row -->
  <tr>
    <th colspan="4">Register Layout Table (First Four 64-bit Registers)</th>
  </tr>
  <tr>
    <td rowspan="8"><code>FER0</code></td>
    <td rowspan="4"><code>HER0</code></td>
    <td rowspan="2"><code>EXR0</code></td>
    <td><code>R0</code></td>
  </tr>
  <tr>
    <td><code>R1</code></td>
  </tr>
  <tr>
    <td rowspan="2"><code>EXR1</code></td>
    <td><code>R2</code></td>
  </tr>
  <tr>
    <td><code>R3</code></td>
  </tr>

  <tr>
    <td rowspan="4"><code>HER1</code></td>
    <td rowspan="2"><code>EXR2</code></td>
    <td><code>R4</code></td>
  </tr>
  <tr>
    <td><code>R5</code></td>
  </tr>
  <tr>
    <td rowspan="2"><code>EXR3</code></td>
    <td><code>R6</code></td>
  </tr>
  <tr>
    <td><code>R7</code></td>
  </tr>

  <!-- FER1 Row -->
  <tr>
    <td rowspan="8"><code>FER1</code></td>
    <td rowspan="4"><code>HER2</code></td>
    <td rowspan="2"><code>EXR4</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td rowspan="2"><code>EXR5</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>

  <tr>
    <td rowspan="4"><code>HER3</code></td>
    <td rowspan="2"><code>EXR6</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td rowspan="2"><code>EXR7</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>

  <!-- FER2 Row -->
  <tr>
    <td rowspan="8"><code>FER2</code></td>
    <td rowspan="4"><code>HER4</code></td>
    <td rowspan="2"><code>-</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td rowspan="2"><code>-</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>

  <tr>
    <td rowspan="4"><code>HER5</code></td>
    <td rowspan="2"><code>-</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td rowspan="2"><code>-</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>

  <!-- FER3 Row -->
  <tr>
    <td rowspan="8"><code>FER3</code></td>
    <td rowspan="4"><code>HER6</code></td>
    <td rowspan="2"><code>-</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td rowspan="2"><code>-</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>

  <tr>
    <td rowspan="4"><code>HER7</code></td>
    <td rowspan="2"><code>-</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td rowspan="2"><code>-</code></td>
    <td><code>-</code></td>
  </tr>
  <tr>
    <td><code>-</code></td>
  </tr>
</table>

## **Assembler Syntax**

### Preprocessor directives

There are three preprocessor directives for Sysdarft Assembly Lanuage.

#### .org

##### Syntax

```asm
    .org [Decimal or Hexadecimal]
```

##### Explanation

`.org`, or origin, defines the code line offset in the whole memory.
Normally origin starts from `0x00`, but some code, like BIOS for example,
loads at `0xC1800`, thus all line markers will be incorrect when offset 
calculation starts from `0x00`.
Thus, using `.org` as a way to define the origin of the code.

#### Usage Example

```asm
    .org 0xC1800
```

#### .equ

##### Syntax

```asm
    .equ '[Search Target]', '[Replacement]'
```

##### Explanation

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

