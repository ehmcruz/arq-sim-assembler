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

// ---------------------------------------



// ---------------------------------------

extern lex_t lex;

static instr_t instrs[ARCH_INSTR_RTYPE_N_OPCODES+ARCH_INSTR_ITYPE_N_OPCODES+1];

static const char *reg_names[] = {
	"r0",
	"r1",
	"r2",
	"r3",
	"r4",
	"r5",
	"r6",
	"r7"
};

extern syntax_tree_t tree;

// ---------------------------------------

const char* get_reg_name (uint16_t code)
{
	return (code < ARCH_NREGS) ? reg_names[code] : NULL;
}

static void lower_mnemonic (instr_t *instr)
{
	char *p;
	for (p=instr->mnemonic; *p; p++)
		*p = tolower(*p);
}

static void load_inst_table ()
{
	uint8_t i, j;

	for (i=0; i<(ARCH_INSTR_RTYPE_N_OPCODES+ARCH_INSTR_ITYPE_N_OPCODES+1); i++) {
		instrs[i].id = INSTR_INVALID;
	}

	i = 0;
	#define SETUP_INSTR(MNEMONIC, OPCODE, TYPE) strcpy(instrs[i].mnemonic, #MNEMONIC); instrs[i].type = TYPE; instrs[i].id = INSTR_##MNEMONIC; instrs[i].opcode = OPCODE; i++;
	#define _MNEMONIC_R_(MNEMONIC, OPCODE) SETUP_INSTR(MNEMONIC, OPCODE, INSTR_TYPE_R)
	#include "mnemonic-r.h"
	#define _MNEMONIC_I_(MNEMONIC, OPCODE) SETUP_INSTR(MNEMONIC, OPCODE, INSTR_TYPE_I)
	#include "mnemonic-i.h"
	#undef _MNEMONIC_R_
	#undef _MNEMONIC_I_
	#undef SETUP_INSTR

	instrs[i].id = INSTR_INVALID;

	assert(i < (ARCH_INSTR_RTYPE_N_OPCODES+ARCH_INSTR_ITYPE_N_OPCODES+1));

	for (i=0; instrs[i].type != INSTR_TYPE_I; i++) {
		lower_mnemonic(instrs+i);
		assert(instrs[i].type == INSTR_TYPE_R);
	}

	for (j=0; instrs[i].id != INSTR_INVALID; i++) {
		lower_mnemonic(instrs+i);
		assert(instrs[i].type == INSTR_TYPE_I);
	}
}

instr_t* get_instr_by_id (instr_id_t id)
{
	uint8_t i;

	for (i=0; instrs[i].id != INSTR_INVALID; i++) {
		if (instrs[i].id == id)
			return instrs+i;
	}

	return NULL;
}

static instr_t* get_instr_by_label (char *label)
{
	uint8_t i;

	for (i=0; instrs[i].id != INSTR_INVALID; i++) {
		if (!strcmp(instrs[i].mnemonic, label))
			return instrs+i;
	}

	return NULL;
}

#define syntax_error_printf(...) { printf("error line %u: ", lex.row); printf(__VA_ARGS__); printf("\n"); exit(1); }

static void token_check_type (lex_token_t *token, lex_token_type_t type)
{
	if (token->type != type) {
		syntax_error_printf("expected %s, given %s", lex_token_str(type), lex_token_str(token->type));
	}
}

static void get_token (lex_token_t *token)
{
	lex_get_token(&lex, token);
	dprintf("fetched token type %s\n", lex_token_str(token->type));
}

static void get_token_expect (lex_token_t *token, lex_token_type_t type)
{
	get_token(token);
	token_check_type(token, type);
}

static char* token_get_label (lex_token_t *token)
{
	return token->data.label;
}

static uint16_t syntax_parse_reg (lex_token_t *token)
{
	uint16_t i;

	get_token_expect(token, LEX_TOKEN_LABEL);
	
	for (i=0; get_reg_name(i) != NULL; i++) {
		if (!strcmp(token_get_label(token), get_reg_name(i)))
			return i;
	}

	syntax_error_printf("unknown register %s", token_get_label(token));

	return 0;
}

static uint16_t syntax_parse_imed (lex_token_t *token, uint32_t bits)
{
	if (token->data.vint > (1 << (int32_t)bits))
		syntax_error_printf("integer %i is too big to fit into %u bits", token->data.vint, bits);

	return token->data.vint;
}

static void syntax_parse_instr_r (lex_token_t *token, instr_t *instr, syntax_tree_node_t *node)
{
	switch (instr->id) {
		case INSTR_LOAD:
			node->data.instr.r_rdest = syntax_parse_reg(token);
			get_token_expect(token, LEX_TOKEN_COMMA);

			get_token_expect(token, LEX_TOKEN_OPEN_BRACKETS);
			node->data.instr.r_rop1 = syntax_parse_reg(token);
			get_token_expect(token, LEX_TOKEN_CLOSE_BRACKETS);

			node->data.instr.r_rop2 = 0;

			dprintf("\tparsed instr: %s %s, [%s]\n", instr->mnemonic, get_reg_name(node->data.instr.r_rdest), get_reg_name(node->data.instr.r_rop1));
			break;

		case INSTR_STORE:
			get_token_expect(token, LEX_TOKEN_OPEN_BRACKETS);
			node->data.instr.r_rop1 = syntax_parse_reg(token);
			get_token_expect(token, LEX_TOKEN_CLOSE_BRACKETS);

			get_token_expect(token, LEX_TOKEN_COMMA);
			node->data.instr.r_rop2 = syntax_parse_reg(token);

			node->data.instr.r_rdest = 0;

			dprintf("\tparsed instr: %s [%s], %s\n", instr->mnemonic, get_reg_name(node->data.instr.r_rop1), get_reg_name(node->data.instr.r_rop2));
			break;

		case INSTR_SYSCALL:
			node->data.instr.r_rdest = 0;
			node->data.instr.r_rop1 = 0;
			node->data.instr.r_rop2 = 0;
			dprintf("\tparsed instr: %s\n", instr->mnemonic);
			break;

		default:
			node->data.instr.r_rdest = syntax_parse_reg(token);
			get_token_expect(token, LEX_TOKEN_COMMA);

			node->data.instr.r_rop1 = syntax_parse_reg(token);
			get_token_expect(token, LEX_TOKEN_COMMA);

			node->data.instr.r_rop2 = syntax_parse_reg(token);

			dprintf("\tparsed instr: %s %s, %s, %s\n", instr->mnemonic, get_reg_name(node->data.instr.r_rdest), get_reg_name(node->data.instr.r_rop1), get_reg_name(node->data.instr.r_rop2));
	}
}

static void syntax_parse_instr_i (lex_token_t *token, instr_t *instr, syntax_tree_node_t *node)
{
	switch (instr->id) {
		case INSTR_JUMP:
			get_token_expect(token, LEX_TOKEN_LABEL);
			node->data.instr.i_uses_label = 1;
			strcpy(node->data.instr.label, token_get_label(token));
			dprintf("\tparsed instr: %s %s\n", instr->mnemonic, node->data.instr.label);
			break;

		case INSTR_JUMP_COND:
			node->data.instr.i_uses_label = 1;
			node->data.instr.i_reg = syntax_parse_reg(token);
			get_token_expect(token, LEX_TOKEN_COMMA);

			get_token_expect(token, LEX_TOKEN_LABEL);
			strcpy(node->data.instr.label, token_get_label(token));
			dprintf("\tparsed instr: %s %s, %s\n", instr->mnemonic, get_reg_name(node->data.instr.i_reg), node->data.instr.label);
			break;

		case INSTR_MOV:
			node->data.instr.i_reg = syntax_parse_reg(token);
			get_token_expect(token, LEX_TOKEN_COMMA);

			// TODO: needs to fix overflow check

			get_token(token);

			if (token->type == LEX_TOKEN_INTEGER) {
				node->data.instr.i_uses_label = 0;
				node->data.instr.i_imed = syntax_parse_imed(token, ARCH_INSTR_ITYPE_IMED_BITS);
				dprintf("\tparsed instr: %s %s, %hi\n", instr->mnemonic, get_reg_name(node->data.instr.i_reg), node->data.instr.i_imed);
			}
			else if (token->type == LEX_TOKEN_LABEL) {
				node->data.instr.i_uses_label = 1;
				strcpy(node->data.instr.label, token_get_label(token));
				dprintf("\tparsed instr: %s %s, %s\n", instr->mnemonic, get_reg_name(node->data.instr.i_reg), node->data.instr.label);
			}
			else
				syntax_error_printf("mov instruction requires a LABEL or INTEGER as second operand");

			break;

		default:
			assert(0);
	}
}

static void syntax_parse_instr (lex_token_t *token, instr_t *instr, syntax_tree_node_t *node)
{
	node->type = SYNTAX_TREE_NODETYPE_INSTR;
	node->data.instr.instr = instr;

	if (instr->type == INSTR_TYPE_R)
		syntax_parse_instr_r(token, instr, node);
	else
		syntax_parse_instr_i(token, instr, node);
}

static void syntax_parse_label (lex_token_t *token, syntax_tree_node_t *node)
{
	node->type = SYNTAX_TREE_NODETYPE_LABEL;
	strcpy(node->data.label, token_get_label(token));
	get_token_expect(token, LEX_TOKEN_COLON);
	dprintf("\tfound label: %s\n", node->data.label);
}

static void syntax_parse_directive (lex_token_t *token, syntax_tree_node_t *node)
{
	int i;

	get_token_expect(token, LEX_TOKEN_LABEL);

	if (!strcmp(token_get_label(token), "data")) {
		node->type = SYNTAX_TREE_NODETYPE_DATA_BLOCK;
		get_token_expect(token, LEX_TOKEN_INTEGER);
		node->data.data = new std::list<uint16_t>;

		if (token->data.vint < 0)
			syntax_error_printf("data block must have a positive number of words, given %i", token->data.vint);

		for (i=0; i<token->data.vint; i++)
			node->data.data->push_back(0);

		dprintf("\tfound data block: %u words\n", node->data.data->size());
	}
	else if (!strcmp(token_get_label(token), "word")) {
		node->type = SYNTAX_TREE_NODETYPE_DATA_BLOCK;
		node->data.data = new std::list<uint16_t>;

		do {
			get_token(token);

			if (token->type != LEX_TOKEN_SEMICOLON) {
				token_check_type(token, LEX_TOKEN_INTEGER);
				node->data.data->push_back(token->data.vint); // TODO: needs to fix overflow check
			}
		} while (token->type != LEX_TOKEN_SEMICOLON);

		dprintf("\tfound word block: %u words\n", node->data.data->size());
	}
	else if (!strcmp(token_get_label(token), "asciiz")) {
		node->type = SYNTAX_TREE_NODETYPE_DATA_BLOCK;
		get_token_expect(token, LEX_TOKEN_STRING);
		node->data.data = new std::list<uint16_t>;

		for (i=0; token->data.string[i] != 0; i++)
			node->data.data->push_back( token->data.string[i] );
		node->data.data->push_back(0);

		dprintf("\tfound asciiz block: %u words (%s)\n", node->data.data->size(), token->data.string );
	}
	else
		syntax_error_printf("unknown directive %s", token_get_label(token));
}

void syntax_analyze ()
{
	lex_token_t token;
	instr_t *instr;
	syntax_tree_node_t *node;

	load_inst_table();

	do {
		get_token(&token);

		if (token.type != LEX_TOKEN_END) {
			node = new syntax_tree_node_t;

			if (token.type == LEX_TOKEN_DOT) {
				syntax_parse_directive(&token, node);
			}
			else {
				token_check_type(&token, LEX_TOKEN_LABEL);
				instr = get_instr_by_label( token_get_label(&token) );

				if (instr != NULL)
					syntax_parse_instr(&token, instr, node);
				else
					syntax_parse_label(&token, node);
			}

			tree.push_back(node);
		}
	} while (token.type != LEX_TOKEN_END);
}
