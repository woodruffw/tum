Design
======

* [Overview](#overview)
* [Registers](#registers)
    + [General Purpose Registers](#general-purpose-registers)
    + [Flag Registers](#flag-registers)
    + [Instruction Pointer](#instruction-pointer)
* [Memory](#memory)
* [Operations](#operations)
    + [Layout](#layout)
    + [Miscellaneous](#miscellaneous)
    + [Arithmetic](#arithmetic)
    + [Control Flow](#control-flow)
    + [Copying](#copying)
    + [Signals](#signals)

## Overview

This document describes the tum architecture. It doesn't describe any internal details,
such as instruction/opcode width or memory size.

All registers, memory addresses, and operations are 64 bits wide **except** for immediate
values, which are only 32 bits wide.

## Registers

### General Purpose Registers

tum has 8 general purpose registers: `gp0` through `gp7`.

### Flag Registers

tum has two flag registers: `af` for arithmetic flags, and `ef` for exception flags.

Arithmetic flags indicate the result of an arithmetic operation.

Exception flags indicate the exception(s) caused by an instruction, if any.

See [Control Flow](#control-flow)
for operations that use the states of the arithmetic flags and exception flags.

### Instruction Pointer

`ip` is the instruction pointer. It contains the address of the instruction currently being decoded.

Since all operations are 64 bytes wide, `ip` always increases in units of 8.

## Memory

## Operations

### Miscellaneous

#### `NOP`

`NOP` performs a no-operation, moving the instruction pointer to the next instruction.

#### `HLT`

`HLT` halts the machine.

#### `CMP`

`CMP REG1, REG2` compares the value of `REG1` to that of `REG2` and sets the arithmetic flags
based on the result.

### Arithmetic

All binary arithmetic operations come in the form `OP REG1, REG2`,
and all unary arithmetic operations come in the form `OP REG`.

The result is always stored in the first register.

#### `ADD`

`ADD` adds two integers.

#### `SUB`

`SUB` subtracts the second integer from the first.

#### `MUL`

`MUL` multiplies two integers.

#### `DIV`

`DIV` divides the first integer by the second.

#### `AND`

`AND` performs a logical AND on two integers.

#### `OR`

`OR` performs a logical OR on two integers.

#### `XOR`

`XOR` performs an exclusive OR on two integers.

#### `NOT`

`NOT REG` performs a logical NOT on `REG` and stores the result in `REG`.

### Control Flow

All control flow operations take come in the form `OP REG`.

If the value of `REG` is not a valid address, the address fault flag is set
in the exception flags.

#### `JMP`

`JMP REG` performs an unconditional jump.

#### `JEQ`

`JEQ REG` performs a conditional jump based on the equality flag.

#### `JLT`

`JLT REG` performs a conditional jump based on the lesser flag.

#### `JLE`

`JLE REG` performs a conditional jump based on the lesser *or* equality flag.

#### `JGT`

`JGT REG` performs a conditional jump based on the greater flag.

#### `JGE`

`JGE REG` performs a conditional jump based on the greater *or* equality flag.

### Copying

#### `MOV`

`MOV REG, IMM` moves the immediate value `IMM` into `REG`. Note that `IMM` is 32 bits wide,
while `REG` is 64 bits.

### Signals

Not implemented yet.
