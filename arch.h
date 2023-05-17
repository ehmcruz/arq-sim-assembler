#ifndef __ARCH_H__
#define __ARCH_H__

#define ARCH_NREGS_LOG             3
#define ARCH_NREGS                 (1 << ARCH_NREGS_LOG)

#define ARCH_INSTR_RTYPE_OPCODE_BITS       6
#define ARCH_INSTR_RTYPE_N_OPCODES         (1 << ARCH_INSTR_RTYPE_OPCODE_BITS)

#define ARCH_INSTR_ITYPE_OPCODE_BITS       2
#define ARCH_INSTR_ITYPE_N_OPCODES         (1 << ARCH_INSTR_ITYPE_OPCODE_BITS)
#define ARCH_INSTR_ITYPE_IMED_BITS         10

#define ARCH_INSTR_TYPE_BIT_POS            15

#endif
