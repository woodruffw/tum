#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdbool.h>

#include "common.h"

static uint32_t lineno = 1;

#define ERRL(fmt, ...) ERR("Line %d: " fmt, lineno, __VA_ARGS__)

/* Writes an encoded instruction to the output stream.
 */
static void emit(isn_t isn)
{
    /* TODO(ww): Make the output configurable.
     */
    fwrite(&isn, sizeof(isn), 1, stdout);
}

/* Converts a stringified register (like "gp0")
 * into its byte flag.
 */
static void regs_to_regf(const char *regs, byte *regf)
{
    if (STREQ(regs, "gp0")) {
        *regf = 1 << 0;
    }
    else if (STREQ(regs, "gp1")) {
        *regf = 1 << 1;
    }
    else if (STREQ(regs, "gp2")) {
        *regf = 1 << 2;
    }
    else if (STREQ(regs, "gp3")) {
        *regf = 1 << 3;
    }
    else if (STREQ(regs, "gp4")) {
        *regf = 1 << 4;
    }
    else if (STREQ(regs, "gp5")) {
        *regf = 1 << 5;
    }
    else if (STREQ(regs, "gp6")) {
        *regf = 1 << 6;
    }
    else if (STREQ(regs, "gp7")) {
        *regf = 1 << 7;
    }
    else {
        ERRL("Unknown register: %s", regs);
        exit(1);
    }
}

/* Parses one register from the given line.
 */
static void parse_one_reg(const char *line, byte *reg)
{
    char *opnd = strchr(line, ' ');

    if (!line || !opnd || !*(opnd + 1)) {
        ERRL("Malformed line: %s", line);
        exit(1);
    }

    /* Advance past the space. */
    opnd += 1;

    regs_to_regf(opnd, reg);
}

/* Parses two registers from the given line.
 */
static void parse_two_regs(const char *line, byte *reg1, byte *reg2)
{
    char *opnds = strchr(line, ' ');

    if (!line || !opnds || !*(opnds + 1)) {
        ERRL("Malformed line: %s", line);
        exit(1);
    }

    /* Advance past the space. */
    opnds += 1;

    /* opnds should now point to a string with format
     * `REG1, REG2`, space optional.
     */
    char *comma = strchr(opnds, ',');

    /* 10 bytes should be more than enough space for our
     * register string (the longest current register name
     * is 3 bytes + NUL).
     */
    char regs[10] = {0};

    if (!comma) {
        ERRL("Malformed line (missing an operand?): %s", line);
        exit(1);
    }

    memcpy(regs, opnds, MIN(comma - opnds, 9));

    LOG("regs (1): %s", regs);

    regs_to_regf(regs, reg1);

    /* Advance past the comma. */
    opnds = comma + 1;

    if (!*opnds) {
        ERRL("Malformed line (missing second operand?): %s", line);
        exit(1);
    }

    /* Exactly one space following the comma is acceptable. */
    if (opnds[0] == ' ') {
        opnds += 1;
    }

    LOG("regs (2): %s", opnds);

    /* What remains should be just a register string. */
    regs_to_regf(opnds, reg2);
}

/* Parses one register and one immediate value from the line.
 */
static void parse_one_reg_one_imm(const char *line, byte *reg, uint32_t *imm)
{
    char *opnd = strchr(line, ' ');

    if (!line || !opnd || !*(opnd + 1)) {
        ERRL("Malformed line: %s", line);
        exit(1);
    }

    /* Advance past the space. */
    opnd += 1;

    char *comma = strchr(opnd, ',');
    char regs[10] = {0};

    if (!comma) {
        ERRL("Malformed line (missing immediate value?): %s", line);
        exit(1);
    }

    memcpy(regs, opnd, MIN(comma - opnd, 9));

    LOG("regs (1): %s", regs);

    regs_to_regf(regs, reg);

    /* Advance past the comma. */
    char *imms = comma + 1;

    /* Exactly one space following the comma is acceptable. */
    if (imms[0] == ' ') {
        imms += 1;
    }

    /* TODO(ww): Support decimal immediates. */
    bool scanned = false;

    scanned = sscanf(imms, "0x%" SCNx32, imm) == 1;

    if (!scanned) {
        scanned = sscanf(imms, "%" SCNd32, imm) == 1;
    }

    if (!scanned) {
        ERRL("Malformed immediate value (not dec/hex?): %s", imms);
        exit(1);
    }
}

/* Evaluates a line of input, emitting its generated instruction.
 */
static void eval(char *line)
{
    isn_t isn = {0};

    line[strlen(line) - 1] = '\0';

    /* Skip empty lines, lines beginning with comment
     * characters.
     */
    if (!line[0] || line[0] == ';' || line[0] == '#') {
        return;
    }

    LOG("eval: %s", line);

    /* TODO(ww): Could probably macro this. */
    if (STRCASEPREFIX(line, "hlt")) {
        isn.op = OP_HLT;
    }
    else if (STRCASEPREFIX(line, "nop")) {
        isn.op = OP_NOP;
    }
    else if (STRCASEPREFIX(line, "cmp")) {
        isn.op = OP_CMP;
        parse_two_regs(line, &isn.reg1, &isn.reg2);
    }
    else if (STRCASEPREFIX(line, "add")) {
        isn.op = OP_ADD;
        parse_two_regs(line, &isn.reg1, &isn.reg2);
    }
    else if (STRCASEPREFIX(line, "sub")) {
        isn.op = OP_SUB;
        parse_two_regs(line, &isn.reg1, &isn.reg2);
    }
    else if (STRCASEPREFIX(line, "mul")) {
        isn.op = OP_MUL;
        parse_two_regs(line, &isn.reg1, &isn.reg2);
    }
    else if (STRCASEPREFIX(line, "div")) {
        isn.op = OP_DIV;
        parse_two_regs(line, &isn.reg1, &isn.reg2);
    }
    else if (STRCASEPREFIX(line, "and")) {
        isn.op = OP_AND;
        parse_two_regs(line, &isn.reg1, &isn.reg2);
    }
    else if (STRCASEPREFIX(line, "or")) {
        isn.op = OP_OR;
        parse_two_regs(line, &isn.reg1, &isn.reg2);
    }
    else if (STRCASEPREFIX(line, "xor")) {
        isn.op = OP_XOR;
        parse_two_regs(line, &isn.reg1, &isn.reg2);
    }
    else if (STRCASEPREFIX(line, "not")) {
        isn.op = OP_NOT;
        parse_one_reg(line, &isn.reg1);
    }
    else if (STRCASEPREFIX(line, "jmp")) {
        isn.op = OP_JMP;
        parse_one_reg(line, &isn.reg1);
    }
    else if (STRCASEPREFIX(line, "jeq")) {
        isn.op = OP_JEQ;
        parse_one_reg(line, &isn.reg1);
    }
    else if (STRCASEPREFIX(line, "jlt")) {
        isn.op = OP_JLT;
        parse_one_reg(line, &isn.reg1);
    }
    else if (STRCASEPREFIX(line, "jle")) {
        isn.op = OP_JLE;
        parse_one_reg(line, &isn.reg1);
    }
    else if (STRCASEPREFIX(line, "jgt")) {
        isn.op = OP_JGT;
        parse_one_reg(line, &isn.reg1);
    }
    else if (STRCASEPREFIX(line, "jge")) {
        isn.op = OP_JGE;
        parse_one_reg(line, &isn.reg1);
    }
    else if (STRCASEPREFIX(line, "mov")) {
        isn.op = OP_MOV;
        parse_one_reg_one_imm(line, &isn.reg1, &isn.imm);
    }
    else if (STRCASEPREFIX(line, "ior")) {
        isn.op = OP_IOR;
        parse_one_reg(line, &isn.reg1);
    }
    else if (STRCASEPREFIX(line, "iow")) {
        isn.op = OP_IOW;
        parse_one_reg(line, &isn.reg1);
    }
    else if (STRCASEPREFIX(line, "sto")) {
        isn.op = OP_STO;
        parse_one_reg_one_imm(line, &isn.reg1, &isn.imm);
    }
    else {
        ERRL("Unknown instruction: %s", line);
        exit(1);
    }

    LOG("encoded isn: " PFUINT, *((uint *) &isn));

    emit(isn);
}

int main()
{
    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, stdin) != -1) {
        eval(line);
        lineno++;
    }

    free(line);
    return 0;
}
