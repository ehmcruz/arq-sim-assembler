#ifndef __LEX_H__
#define __LEX_H__

#include <stdint.h>
#include <stdio.h>

#include "asm.h"

enum lex_token_type_t {
	#define _TOKEN_TYPE_(TOKEN) LEX_TOKEN_##TOKEN,
	#include "lex-token-types.h"
	#undef _TOKEN_TYPE_
	LEX_TOKEN_MAX
};

struct lex_token_t {
	union {
		char label[MAX_LABEL_LENGTH];
		char string[MAX_LABEL_LENGTH];
		int vint;
	} data;
	
	lex_token_type_t type;
};

struct lex_t {
	char *fname;
	FILE *fp;
	char *buffer;
	uint32_t bsize;
	uint32_t bpos;

	uint32_t col;
	uint32_t row;
	uint32_t has_white_space_token; // default: 0
	uint32_t merge_white_space; // default: 1
	uint32_t has_newline_token; // default: 0
};

void lex_init (lex_t *lex, char *fname); // initialize with default configuration
void lex_get_token (lex_t *lex, lex_token_t *token);

const char* lex_token_str (lex_token_type_t type);

#endif
