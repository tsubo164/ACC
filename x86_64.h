#ifndef X86_64_H
#define X86_64_H

#include <stdio.h>

enum operand_size {
    I0, I8, I16, I32, I64
};

enum operand_00 {
    A__00,   AL_00,   AX_00,   EAX_00,  RAX_00,
    B__00,   BL_00,   BX_00,   EBX_00,  RBX_00,
    C__00,   CL_00,   CX_00,   ECX_00,  RCX_00,
    D__00,   DL_00,   DX_00,   EDX_00,  RDX_00,
    DI__00,  DIL_00,  DI_00,   EDI_00,  RDI_00,
    SI__00,  SIL_00,  SI_00,   ESI_00,  RSI_00,
    BP__00,  BPL_00,  BP_00,   EBP_00,  RBP_00,
    SP__00,  SPL_00,  SP_00,   ESP_00,  RSP_00,
    R8__00,  R8B_00,  R8W_00,  R8D_00,  R8_00,
    R9__00,  R9B_00,  R9W_00,  R9D_00,  R9_00,
    R10__00, R10B_00, R10W_00, R10D_00, R10_00,
    R11__00, R11B_00, R11W_00, R11D_00, R11_00,
    R12__00, R12B_00, R12W_00, R12D_00, R12_00,
    R13__00, R13B_00, R13W_00, R13D_00, R13_00,
    R14__00, R14B_00, R14W_00, R14D_00, R14_00,
    R15__00, R15B_00, R15W_00, R15D_00, R15_00,
    OPR_IMM_00,
    OPR_MEM_00,
    OPR_LBL_00,
    OPR_SYM_00
};

enum opecode_00 {
    MOV_00, MOVSB_00, MOVSW_00, MOVSL_00, MOVZB_00, MOVZW_00,
    ADD_00, SUB_00, IMUL_00, DIV_00, IDIV_00,
    SHL_00, SHR_00, SAR_00,
    OR_00, XOR_00, AND_00, NOT_00, CMP_00,

    LEA_00, POP_00, PUSH_00,
    CALL_00, RET_00, JE_00, JNE_00, JMP_00,
    SETE_00, SETNE_00, SETL_00, SETG_00, SETLE_00, SETGE_00,
    CLTD_00, CQTO_00
};

enum operand_00 arg_reg_00(int n);
enum operand_00 imm_00(long val);
enum operand_00 mem_00(enum operand_00 reg, int offset);
enum operand_00 symb_00(const char *sym, int id);

enum operand_00 label_00(const char *label_name, int id);
extern void code1_00(FILE *fp, enum opecode_00 op);
extern void code2_00(FILE *fp, enum opecode_00 op, enum operand_00 oper1);
extern void code3_00(FILE *fp, enum opecode_00 op, enum operand_00 oper1, enum operand_00 oper2);

#endif /* _H */
