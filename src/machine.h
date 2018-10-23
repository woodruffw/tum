#pragma once

/* The size of the address space. */
#define MEMORY_SIZE (1024 * 1024 * 16)

/* Indicates whether the given address is in the address space.
 * Note that this evaluates to false if the address is too small
 * to reference an instruction.
 * TODO: Maybe rename this, to avoid confusing with the sense
 * of "valid" used in GUARD_ADDR below.
 */
#define VALID_ADDR(addr) ((addr) + 7 < MEMORY_SIZE)

/* Extracts the opcode from an instruction.
 * See the decoding documentation under fetch() in machine.c.
 */
#define ISN2OP(isn) ((isn & 0x00000000FFFF0000) >> 16)

/* Extracts the first (destination) register from an
 * instruction.
 * See the decoding documentation under fetch() in machine.c.
 */
#define ISN2REG1(isn) (flag_to_regp((isn & 0x0000000000FF00) >> 8))

/* Extracts the second register from an
 * instruction.
 * See the decoding documentation under fetch() in machine.c.
 */
#define ISN2REG2(isn) (flag_to_regp(isn & 0x000000000000FF))

/* Extracts the immediate value from an instruction.
 */
#define ISN2IMM(isn) ((isn & 0xFFFFFFFF00000000) >> 32)

/* Extract the value in memory given by a register.
 */
#define REG_DEREF(reg) (*((uint *) (machine.mem + (reg))))

/* Guards a pointer to a register.
 */
#define GUARD_REGP(reg, ...)            \
    if (!(reg)) {                       \
        machine.ctx.ef |= EF_REG_FAULT; \
        return ##__VA_ARGS__;           \
    }

/* Guards a value that needs to be address-aligned.
 */
#define GUARD_ALIGN(val, ...)            \
    if ((val) % 8) {                     \
        machine.ctx.ef |= EF_ADDR_ALIGN; \
        return ##__VA_ARGS__;            \
    }

/* Guards a value that needs to be a valid address,
 * where "valid" means both aligned and within the
 * address space.
 */
#define GUARD_ADDR(val, ...)             \
    GUARD_ALIGN(val, ##__VA_ARGS__);     \
    if (!VALID_ADDR(val)) {              \
        machine.ctx.ef |= EF_ADDR_FAULT; \
        return ##__VA_ARGS__;            \
    }

/* Arithmetic flags.
 */
#define AF_EQUAL (1 << 0)
#define AF_GREATER (1 << 1)
#define AF_LESSER (1 << 2)

/* Exception flags.
 */
#define EF_ADDR_FAULT ((uint) 1 << 63)
#define EF_UNK_ISN ((uint) 1 << 62)
#define EF_REG_FAULT ((uint) 1 << 61)
#define EF_ADDR_ALIGN ((uint) 1 << 60)

/* The machine's register context. */
typedef struct
{
    /* General purpose registers. */
    uint gp0;
    uint gp1;
    uint gp2;
    uint gp3;
    uint gp4;
    uint gp5;
    uint gp6;
    uint gp7;

    /* Flag registers. */
    uint af;
    uint ef;

    /* Instruction pointer. */
    uint ip;
} context_t;

/* The machine's memory. */
typedef byte memory_t[MEMORY_SIZE];

/* The machine's whole state. */
typedef struct
{
    context_t ctx;
    memory_t mem;
} machine_t;

