#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>

#include "common.h"
#include "machine.h"

static machine_t machine;
static bool halt;

static void dump_context()
{
    fprintf(stderr, "gp0=" PFUINT " gp1=" PFUINT " gp2=" PFUINT " gp3=" PFUINT "\n"
            "gp4=" PFUINT " gp5=" PFUINT " gp6=" PFUINT " gp7=" PFUINT "\n"
            "af=" PFUINT " ef=" PFUINT " ip=" PFUINT "\n",
            machine.ctx.gp0, machine.ctx.gp1, machine.ctx.gp2, machine.ctx.gp3,
            machine.ctx.gp4, machine.ctx.gp5, machine.ctx.gp6, machine.ctx.gp7,
            machine.ctx.af, machine.ctx.ef, machine.ctx.ip);
}

static uint *flag_to_regp(byte flag)
{
    uint *reg = NULL;

    switch (flag) {
        case 1:
            reg = &(machine.ctx.gp0);
            break;
        case 2:
            reg = &(machine.ctx.gp1);
            break;
        case 4:
            reg = &(machine.ctx.gp2);
            break;
        case 8:
            reg = &(machine.ctx.gp3);
            break;
        case 16:
            reg = &(machine.ctx.gp4);
            break;
        case 32:
            reg = &(machine.ctx.gp5);
            break;
        case 64:
            reg = &(machine.ctx.gp6);
            break;
        case 128:
            reg = &(machine.ctx.gp7);
            break;
    }

    return reg;
}

static void op_cmp(uint *reg1, uint *reg2)
{
    GUARD_REGP(reg1);
    GUARD_REGP(reg2);

    if (*reg1 > *reg2) {
        machine.ctx.af |= AF_GREATER;
    }
    else if (*reg1 < *reg2) {
        machine.ctx.af |= AF_LESSER;
    }
    else {
        machine.ctx.af |= AF_EQUAL;
    }
}

static void op_add(uint *reg1, uint *reg2)
{
    GUARD_REGP(reg1);
    GUARD_REGP(reg2);

    *reg1 = *reg1 + *reg2;
}

static void op_sub(uint *reg1, uint *reg2)
{
    GUARD_REGP(reg1);
    GUARD_REGP(reg2);

    *reg1 = *reg1 - *reg2;
}

static void op_mul(uint *reg1, uint *reg2)
{
    GUARD_REGP(reg1);
    GUARD_REGP(reg2);

    *reg1 = *reg1 * *reg2;
}

static void op_div(uint *reg1, uint *reg2)
{
    GUARD_REGP(reg1);
    GUARD_REGP(reg2);

    *reg1 = *reg1 / *reg2;
}

static void op_and(uint *reg1, uint *reg2)
{
    GUARD_REGP(reg1);
    GUARD_REGP(reg2);

    *reg1 = *reg1 & *reg2;
}

static void op_or(uint *reg1, uint *reg2)
{
    GUARD_REGP(reg1);
    GUARD_REGP(reg2);

    *reg1 = *reg1 | *reg2;
}

static void op_xor(uint *reg1, uint *reg2)
{
    GUARD_REGP(reg1);
    GUARD_REGP(reg2);

    *reg1 = *reg1 ^ *reg2;
}

static void op_not(uint *reg)
{
    GUARD_REGP(reg);

    *reg = ~*reg;
}

static void op_jmp(uint *reg)
{
    GUARD_REGP(reg);

    machine.ctx.ip = *reg;
}

static void op_jeq(uint *reg)
{
    GUARD_REGP(reg);

    if (machine.ctx.af & AF_EQUAL) {
        machine.ctx.ip = *reg;
    }
}

static void op_jlt(uint *reg)
{
    GUARD_REGP(reg);

    if (machine.ctx.af & AF_LESSER) {
        machine.ctx.ip = *reg;
    }
}

static void op_jle(uint *reg)
{
    GUARD_REGP(reg);

    if (machine.ctx.af & (AF_LESSER | AF_EQUAL)) {
        machine.ctx.ip = *reg;
    }
}

static void op_jgt(uint *reg)
{
    GUARD_REGP(reg);

    if (machine.ctx.af & AF_GREATER) {
        machine.ctx.ip = *reg;
    }
}

static void op_jge(uint *reg)
{
    GUARD_REGP(reg);

    if (machine.ctx.af & (AF_LESSER | AF_EQUAL)) {
        machine.ctx.ip = *reg;
    }
}

static void op_mov(uint *reg, uint imm)
{
    GUARD_REGP(reg);
    *reg = imm;
}

static void op_ior(uint *reg)
{
    GUARD_REGP(reg);
    *reg = getchar();
}

static void op_iow(uint *reg)
{
    GUARD_REGP(reg);
    putchar((int) *reg);
}

static void tick()
{
    /* Fetch the next instruction via the instruction pointer,
     * faulting if the IP isn't valid.
     */

    if (!VALID_ADDR(machine.ctx.ip)) {
        /* The instruction pointer isn't in our address space.
         * Set the exception flag to indicate an address fault,
         * and halt.
         */
        machine.ctx.ef |= EF_ADDR_FAULT;
        return;
    }

    /* TODO: Use isn_t here and below.
     */
    GUARD_ALIGN(machine.ctx.ip);
    uint isn = REG_DEREF(machine.ctx.ip);

    machine.ctx.ip += 8;

    LOG("Fetch: isn: " PFUINT, isn);

    /* Decode and execute the instruction, faulting if it
     * doesn't decode to a valid operation.
     *
     * The higher 32 bits contain the immediate value for
     * the instruction, if any.
     *
     * For the lower 32 bits:
     * The high 16 bits contain the opcode, and the low
     * bits contain the operands. The high byte of the
     * lower word is always the destination, if the
     * operation uses one.
     *
     * Visualized:
     *
     * immediate value     | opcode    | registers
     * -------------------------------------------
     * IM   IM   IM   IM   | OP   OP   | RG   RG
     * -------------------------------------------
     * 63                 47          31         0
     */

    uint opcode = ISN2OP(isn);

    LOG("opcode: " PFUINT, opcode);

    switch (opcode) {
        case OP_HLT:
            LOG("decoded OP_HLT, halting");
            halt = 1;
            break;
        case OP_NOP:
            break;
        case OP_CMP:
            op_cmp(ISN2REG1(isn), ISN2REG2(isn));
            break;
        case OP_ADD:
            op_add(ISN2REG1(isn), ISN2REG2(isn));
            break;
        case OP_SUB:
            op_sub(ISN2REG1(isn), ISN2REG2(isn));
            break;
        case OP_MUL:
            op_mul(ISN2REG1(isn), ISN2REG2(isn));
            break;
        case OP_DIV:
            op_div(ISN2REG1(isn), ISN2REG2(isn));
            break;
        case OP_AND:
            op_and(ISN2REG1(isn), ISN2REG2(isn));
            break;
        case OP_OR:
            op_or(ISN2REG1(isn), ISN2REG2(isn));
            break;
        case OP_XOR:
            op_xor(ISN2REG1(isn), ISN2REG2(isn));
            break;
        case OP_NOT:
            op_not(ISN2REG1(isn));
            break;
        case OP_JMP:
            op_jmp(ISN2REG1(isn));
            break;
        case OP_JEQ:
            op_jeq(ISN2REG1(isn));
            break;
        case OP_JLT:
            op_jlt(ISN2REG1(isn));
            break;
        case OP_JLE:
            op_jle(ISN2REG1(isn));
            break;
        case OP_JGT:
            op_jgt(ISN2REG1(isn));
            break;
        case OP_JGE:
            op_jge(ISN2REG1(isn));
            break;
        case OP_MOV:
            op_mov(ISN2REG1(isn), ISN2IMM(isn));
            break;
        case OP_IOR:
            op_ior(ISN2REG1(isn));
            break;
        case OP_IOW:
            op_iow(ISN2REG1(isn));
            break;
        default:
            LOG("unknown opcode: " PFUINT, opcode);
            machine.ctx.ef |= EF_UNK_ISN;
            break;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: tmachine <exe>\n");
        return 0;
    }

    int fd;
    if ((fd = open(argv[1], O_RDONLY)) < 0) {
        perror("read");
        return errno;
    }

    if (!read(fd, machine.mem, MEMORY_SIZE)) {
        perror("read");
        return errno;
    }

    close(fd);

    while (!halt) {
        LOG("BOT: ip=" PFUINT, machine.ctx.ip);
        tick();
        LOG("EOT: ip=" PFUINT "\n\n", machine.ctx.ip);

        if (machine.ctx.ef) {
            LOG("FAULT");
            halt = true;
        }
    }

    dump_context();

    return 0;
}
