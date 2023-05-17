#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <list>

#include "lex.h"
#include "syntax.h"
#include "arch.h"
#include "asm.h"

static const char start_label[] = "_start";

#define linker_error_printf(...) { printf("linker error: "); printf(__VA_ARGS__); printf("\n"); exit(1); }

/*
	virtual addr 0 is invalid and causes a trap to OS
	virtual addr 1 contains the jump instruction to the _start label
*/

static void linker_calculate_vaddrs (syntax_tree_t *tree)
{
	syntax_tree_t::iterator it;
	syntax_tree_node_t *node;
	uint16_t next_vaddr, total_words;

	next_vaddr = 2;

	for (it=tree->begin(); it != tree->end(); it++) {
		node = *it;

		node->vaddr = next_vaddr;

		switch (node->type) {
			case SYNTAX_TREE_NODETYPE_DATA_BLOCK:
				node->words = node->data.data->size();
				break;

			case SYNTAX_TREE_NODETYPE_LABEL:
				node->words = 0;
				dprintf("attributed vaddr %hu to label %s\n", node->vaddr, node->data.label);
				break;

			case SYNTAX_TREE_NODETYPE_INSTR:
				node->words = 1;
				break;

			default:
				assert(0);
		}

		next_vaddr += node->words;
	}
}

static syntax_tree_node_t* search_label_node (syntax_tree_t *tree, char *label)
{
	syntax_tree_t::iterator it;
	syntax_tree_node_t *node;

	for (it=tree->begin(); it != tree->end(); it++) {
		node = *it;

		if (node->type == SYNTAX_TREE_NODETYPE_LABEL && !strcmp(node->data.label, label))
			return node;
	}

	return NULL;
}

static void linker_fill_place_holders (syntax_tree_t *tree)
{
	syntax_tree_t::iterator it;
	syntax_tree_node_t *node, *target;
	
	for (it=tree->begin(); it != tree->end(); it++) {
		node = *it;

		switch (node->type) {
			case SYNTAX_TREE_NODETYPE_INSTR:
				if (node->data.instr.instr->type == INSTR_TYPE_I && node->data.instr.i_uses_label == 1) {
					target = search_label_node(tree, node->data.instr.label);

					if (!target)
						linker_error_printf("could not locate label %s", node->data.instr.label);

					node->data.instr.i_imed = target->vaddr;

					dprintf("filled placeholder for %s instruction at vaddr %hu, target is %hu\n", node->data.instr.instr->mnemonic, node->vaddr, target->vaddr);
				}
				break;
		}
	}
}

static uint16_t encode_rtype_instruction (uint16_t opcode, uint16_t r_dest, uint16_t r_op1, uint16_t r_op2)
{
	uint16_t instr = 0x0000;
	uint16_t pos = 0;

	dprintf("encoding instr rtype: opcode=%hu reg_dest=%hu r_op1=%hu r_op1=%hu\n", opcode, r_dest, r_op1, r_op2);

	instr |= r_op2;
	pos += ARCH_NREGS_LOG;

	instr |= r_op1 << pos;
	pos += ARCH_NREGS_LOG;

	instr |= r_dest << pos;
	pos += ARCH_NREGS_LOG;

	instr |= opcode << pos;
	pos += ARCH_INSTR_RTYPE_OPCODE_BITS;

	return instr;
}

static uint16_t encode_itype_instruction (uint16_t opcode, uint16_t reg, uint16_t imed)
{
	uint16_t instr = 1 << ARCH_INSTR_TYPE_BIT_POS;
	uint16_t pos = 0;

	dprintf("encoding instr itype: opcode=%hu reg=%hu imed=%hu\n", opcode, reg, imed);

	instr |= imed;
	pos += ARCH_INSTR_ITYPE_IMED_BITS;

	instr |= reg << pos;
	pos += ARCH_NREGS_LOG;

	instr |= opcode << pos;

	return instr;
}

static void write_instruction (FILE *fp, uint16_t encoded_instr)
{
	fwrite(&encoded_instr, sizeof(uint16_t), 1, fp);
}

void linker_generate_binary (syntax_tree_t *tree, char *fname_out)
{
	FILE *fp;
	syntax_tree_node_t *node;
	syntax_tree_t::iterator it;
	uint16_t encoded_instr;
	instr_t *instr;

	linker_calculate_vaddrs(tree);
	linker_fill_place_holders(tree);

	fp = fopen(fname_out, "w");
	if (!fp) {
		printf("cannot open output file %s\n", fname_out);
		exit(1);
	}

	write_instruction(fp, 0);

	node = search_label_node(tree, (char*)start_label);

	if (!node)
		linker_error_printf("could not locate start label %s, which should point to the first instruction to be executed", start_label);

	// encode a jump instruction to the _start label
	instr = get_instr_by_id(INSTR_JUMP);
	assert(instr != NULL);
	encoded_instr = encode_itype_instruction(instr->opcode, 0, node->vaddr);
	write_instruction(fp, encoded_instr);

	for (it=tree->begin(); it != tree->end(); it++) {
		node = *it;

		switch (node->type) {
			case SYNTAX_TREE_NODETYPE_DATA_BLOCK: {
				std::list<uint16_t>::iterator it;

				for (it=node->data.data->begin(); it != node->data.data->end(); it++) {
					uint16_t data;

					data = *it;
					fwrite(&data, sizeof(uint16_t), 1, fp);
				}

				break;
			}

			case SYNTAX_TREE_NODETYPE_LABEL:
				break;

			case SYNTAX_TREE_NODETYPE_INSTR:
				if (node->data.instr.instr->type == INSTR_TYPE_R) {
					encoded_instr = encode_rtype_instruction(node->data.instr.instr->opcode, node->data.instr.r_rdest, node->data.instr.r_rop1, node->data.instr.r_rop2);
					write_instruction(fp, encoded_instr);
				}
				else {
					encoded_instr = encode_itype_instruction(node->data.instr.instr->opcode, node->data.instr.i_reg, node->data.instr.i_imed);
					write_instruction(fp, encoded_instr);
				}
				break;

			default:
				assert(0);
		}
	}

	fclose(fp);

	printf("binary %s generated! yes!\n", fname_out);
}
