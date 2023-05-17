#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include <list>

#include "lex.h"
#include "asm.h"

enum instr_type_t {
	INSTR_TYPE_R,
	INSTR_TYPE_I
};

enum instr_id_t {
	#define _MNEMONIC_R_(MNEMONIC, OPCODE) INSTR_##MNEMONIC,
	#include "mnemonic-r.h"
	#define _MNEMONIC_I_(MNEMONIC, OPCODE) _MNEMONIC_R_(MNEMONIC, OPCODE)
	#include "mnemonic-i.h"
	#undef _MNEMONIC_R_
	#undef _MNEMONIC_I_

	INSTR_INVALID
};

struct instr_t {
	char mnemonic[20];
	instr_type_t type;
	instr_id_t id;
	uint16_t opcode;
};

enum syntax_tree_nodetype_t {
	SYNTAX_TREE_NODETYPE_DATA_BLOCK,
	SYNTAX_TREE_NODETYPE_LABEL,
	SYNTAX_TREE_NODETYPE_INSTR
};

struct syntax_tree_node_t {
	syntax_tree_nodetype_t type;

	uint16_t words; // used by linker
	uint16_t vaddr; // used by the linker

	union {
		struct {
			instr_t *instr;
			uint16_t r_rdest;
			uint16_t r_rop1;
			uint16_t r_rop2;
			uint16_t i_reg;
			uint16_t i_imed;
			uint8_t i_uses_label;
			char label[MAX_LABEL_LENGTH];
		} instr;

		char label[MAX_LABEL_LENGTH];

		std::list<uint16_t> *data;
	} data;
};

typedef std::list<syntax_tree_node_t*> syntax_tree_t;

instr_t* get_instr_by_id (instr_id_t id);
void syntax_analyze ();

#endif
