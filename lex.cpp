#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include "lex.h"

enum lex_state_t {
	LEX_STATE_INIT,
	LEX_STATE_WHITE_SPACE,
	LEX_STATE_CHECK_MINUS_OR_INTEGER,
	LEX_STATE_INTEGER,
	LEX_STATE_LABEL,
	LEX_STATE_STRING,
	LEX_STATE_COMMENT,
	LEX_STATE_END
};

static const char *token_str[] = {
#define _TOKEN_TYPE_(TOKEN) #TOKEN,
#include "lex-token-types.h"
#undef _TOKEN_TYPE_
};

const char* lex_token_str (lex_token_type_t type)
{
	assert(type < LEX_TOKEN_MAX);
	return (const char*)token_str[type];
}

static void load_file (lex_t *lex)
{
	lex->fp = fopen(lex->fname, "r");

	if (!lex->fp) {
		printf("cannot open file %s\n", lex->fname);
		exit(1);
	}

	fseek(lex->fp, 0, SEEK_END);
	lex->bsize = ftell(lex->fp);
	printf("file %s has %u bytes\n", lex->fname, lex->bsize);
	lex->buffer = (char*)malloc(lex->bsize + 1);
	assert(lex->buffer != NULL);
	rewind(lex->fp);

	if (fread(lex->buffer, 1, lex->bsize, lex->fp) != lex->bsize) {
		printf("error loading file %s\n", lex->fname);
		exit(1);
	}

	lex->buffer[ lex->bsize ] = 0;
	lex->bpos = 0;
}

static char io_fgetc_no_inc (lex_t *lex)
{
	return lex->buffer[ lex->bpos ];
}

static void io_inc (lex_t *lex)
{
	lex->bpos++;
}

void lex_init (lex_t *lex, char *fname)
{
	lex->fname = fname;
	load_file(lex);

	lex->col = 1;
	lex->row = 1;
	lex->has_white_space_token = 0;
	lex->merge_white_space = 1;
	lex->has_newline_token = 0;
}

void lex_get_token (lex_t *lex, lex_token_t *token)
{
	lex_state_t state;
	char c;
	char *p;
	int number_is_negative;
	
	state = LEX_STATE_INIT;
	
	while (state != LEX_STATE_END) {
		c = io_fgetc_no_inc(lex);
		
		switch (state) {
			case LEX_STATE_INIT:
				if (c == 0) {
					token->type = LEX_TOKEN_END;
					state = LEX_STATE_END;
				}
				else if (c == '#') {
					io_inc(lex);
					state = LEX_STATE_COMMENT;
				}
				else if (c == ' ' || c == '\t') {
					io_inc(lex);
					if (lex->has_white_space_token) {
						if (lex->merge_white_space)
							state = LEX_STATE_WHITE_SPACE;
						else {
							token->type = LEX_TOKEN_WHITE_SPACE;
							state = LEX_STATE_END;
						}
					}
				}
				else if (iswdigit(c)) {
					p = token->data.label;
					// *p = c;
					// p++;
					state = LEX_STATE_INTEGER;
					number_is_negative = 0;
					// io_inc(lex);
				}
				else if (iswalpha(c) || c == '_') {
					p = token->data.label;
					*p = c;
					p++;
					state = LEX_STATE_LABEL;
					io_inc(lex);
				}
				else if (c == '\n') {
					io_inc(lex);
					lex->row++;
					if (lex->has_newline_token) {
						token->type = LEX_TOKEN_NEWLINE;
						state = LEX_STATE_END;
					}
				}
				else if (c == '-') {
					io_inc(lex);
					state = LEX_STATE_CHECK_MINUS_OR_INTEGER;
				}
				else if (c == '[') {
					io_inc(lex);
					token->type = LEX_TOKEN_OPEN_BRACKETS;
					state = LEX_STATE_END;
				}
				else if (c == ']') {
					io_inc(lex);
					token->type = LEX_TOKEN_CLOSE_BRACKETS;
					state = LEX_STATE_END;
				}
				else if (c == ':') {
					io_inc(lex);
					token->type = LEX_TOKEN_COLON;
					state = LEX_STATE_END;
				}
				else if (c == ',') {
					io_inc(lex);
					token->type = LEX_TOKEN_COMMA;
					state = LEX_STATE_END;
				}
				else if (c == '.') {
					io_inc(lex);
					token->type = LEX_TOKEN_DOT;
					state = LEX_STATE_END;
				}
				else if (c == ';') {
					io_inc(lex);
					token->type = LEX_TOKEN_SEMICOLON;
					state = LEX_STATE_END;
				}
				else if (c == '"') {
					p = token->data.string;
					state = LEX_STATE_STRING;
					io_inc(lex);
				}
				else {
					printf("lex error line %u char %c\n", lex->row, c);
					exit(1);
				}
				break;

			case LEX_STATE_COMMENT:
				io_inc(lex);
				if (c == '\n') {
					lex->row++;
					if (lex->has_newline_token) {
						token->type = LEX_TOKEN_NEWLINE;
						state = LEX_STATE_END;
					}
					else
						state = LEX_STATE_INIT;
				}
				break;

			case LEX_STATE_CHECK_MINUS_OR_INTEGER:
				if (iswdigit(c)) {
					p = token->data.label;
					*p = '-';
					p++;
					state = LEX_STATE_INTEGER;
					number_is_negative = 1;
				}
				else {
					token->type = LEX_TOKEN_MINUS;
					state = LEX_STATE_END;
				}
				break;
			
			case LEX_STATE_WHITE_SPACE:
				if (c == ' ' || c == '\t')
					io_inc(lex);
				else {
					token->type = LEX_TOKEN_WHITE_SPACE;
					state = LEX_STATE_END;
				}
				break;
			
			case LEX_STATE_INTEGER:
				if (iswdigit(c)) {
					*p = c;
					p++;
					io_inc(lex);
				}
				else {
					*p = 0;
					token->data.vint = atoi(token->data.label);
					token->type = LEX_TOKEN_INTEGER;
					state = LEX_STATE_END;
				}
				break;
			
			case LEX_STATE_LABEL:
				if (iswalnum(c) || c == '_') {
					*p = c;
					p++;
					io_inc(lex);
				}
				else {
					*p = 0;
					token->type = LEX_TOKEN_LABEL;
					state = LEX_STATE_END;
				}
				break;
			
			case LEX_STATE_STRING:
				if (c != '"') {
					*p = c;
					p++;
					io_inc(lex);
				}
				else {
					*p = 0;
					io_inc(lex);
					token->type = LEX_TOKEN_STRING;
					state = LEX_STATE_END;
				}
				break;

			default:
				assert(0);
		}
	}
}

