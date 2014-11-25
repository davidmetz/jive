/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_INSTRUCTIONSET_H
#define JIVE_BACKEND_I386_INSTRUCTIONSET_H

#include <jive/arch/instruction.h>

struct jive_instructionset;

extern const jive_instruction_class jive_i386_instr_ret;

extern const jive_instruction_class jive_i386_instr_int_load_imm;
extern const jive_instruction_class jive_i386_instr_int_load32_disp;
extern const jive_instruction_class jive_i386_instr_int_store32_disp;

extern const jive_instruction_class jive_i386_instr_int_add;
extern const jive_instruction_class jive_i386_instr_int_sub;
extern const jive_instruction_class jive_i386_instr_int_and;
extern const jive_instruction_class jive_i386_instr_int_or;
extern const jive_instruction_class jive_i386_instr_int_xor;
extern const jive_instruction_class jive_i386_instr_int_mul;
extern const jive_instruction_class jive_i386_instr_int_sdiv;
extern const jive_instruction_class jive_i386_instr_int_udiv;
extern const jive_instruction_class jive_i386_instr_int_shl;
extern const jive_instruction_class jive_i386_instr_int_shr;
extern const jive_instruction_class jive_i386_instr_int_ashr;
extern const jive_instruction_class jive_i386_instr_int_mul_expand_signed;
extern const jive_instruction_class jive_i386_instr_int_mul_expand_unsigned;

extern const jive_instruction_class jive_i386_instr_int_add_immediate;
extern const jive_instruction_class jive_i386_instr_int_sub_immediate;
extern const jive_instruction_class jive_i386_instr_int_and_immediate;
extern const jive_instruction_class jive_i386_instr_int_or_immediate;
extern const jive_instruction_class jive_i386_instr_int_xor_immediate;
extern const jive_instruction_class jive_i386_instr_int_mul_immediate;
extern const jive_instruction_class jive_i386_instr_int_shl_immediate;
extern const jive_instruction_class jive_i386_instr_int_shr_immediate;
extern const jive_instruction_class jive_i386_instr_int_ashr_immediate;

extern const jive_instruction_class jive_i386_instr_int_neg;
extern const jive_instruction_class jive_i386_instr_int_not;

extern const jive_instruction_class jive_i386_instr_int_transfer;

extern const jive_instruction_class jive_i386_instr_call;
extern const jive_instruction_class jive_i386_instr_call_reg;

extern const jive_instruction_class jive_i386_instr_int_cmp;
extern const jive_instruction_class jive_i386_instr_int_cmp_immediate;

extern const jive_instruction_class jive_i386_instr_int_jump_sless;
extern const jive_instruction_class jive_i386_instr_int_jump_uless;
extern const jive_instruction_class jive_i386_instr_int_jump_slesseq;
extern const jive_instruction_class jive_i386_instr_int_jump_ulesseq;
extern const jive_instruction_class jive_i386_instr_int_jump_equal;
extern const jive_instruction_class jive_i386_instr_int_jump_notequal;
extern const jive_instruction_class jive_i386_instr_int_jump_sgreater;
extern const jive_instruction_class jive_i386_instr_int_jump_ugreater;
extern const jive_instruction_class jive_i386_instr_int_jump_sgreatereq;
extern const jive_instruction_class jive_i386_instr_int_jump_ugreatereq;
extern const jive_instruction_class jive_i386_instr_jump;

extern const jive_instruction_class jive_i386_instr_fp_load_disp;

extern const jive_instruction_class jive_i386_instr_sse_load32_disp;
extern const jive_instruction_class jive_i386_instr_sse_load_abs;
extern const jive_instruction_class jive_i386_instr_sse_store32_disp;

extern const jive_instruction_class jive_i386_instr_sse_xor;

extern const jive_instruction_class jive_i386_instr_float_add;
extern const jive_instruction_class jive_i386_instr_float_sub;
extern const jive_instruction_class jive_i386_instr_float_mul;
extern const jive_instruction_class jive_i386_instr_float_div;

extern const jive_instruction_class jive_i386_instr_float_cmp;
extern const jive_instruction_class jive_i386_instr_float_transfer;

extern const struct jive_instructionset jive_i386_instructionset;

#endif
