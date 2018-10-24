#pragma once

#define MIN(a, b) ((a) <= (b) ? (a) : (b))

/* A convenience hex printf-specifier for uint64_t. */
#define PFUINT "0x%016" PRIx64

#ifdef DEBUG
#   define LOG(fmt, ...) (fprintf(stderr, "%s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__))
#else
#   define LOG(...)
#endif

#define ERR(fmt, ...) (fprintf(stderr, fmt "\n", ##__VA_ARGS__))

#define STREQ(str1, str2) (!(strcmp((str1), (str2))))
#define STRCASEPREFIX(str, prefix) (!strncasecmp((str), (prefix), strlen(prefix)))

/* A convenience typedef for the system width. */
typedef uint64_t uint;

/* Another convenience typedef, for the memory resolution. */
typedef uint8_t byte;

enum opcodes
{
    OP_HLT = 0,
    OP_NOP,
    OP_CMP,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_NOT,
    OP_JMP,
    OP_JEQ,
    OP_JNE,
    OP_JLT,
    OP_JLE,
    OP_JGT,
    OP_JGE,
    OP_MOV,
    OP_STO,
    OP_LOA,
    OP_SIP,
    OP_IOR,
    OP_IOW,
};

typedef struct
{
    byte reg2;
    byte reg1; /* The destination register, by convention. */
    uint16_t op;
    uint32_t imm;
} isn_t;
