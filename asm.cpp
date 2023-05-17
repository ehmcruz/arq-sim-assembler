#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lex.h"
#include "syntax.h"
#include "linker.h"
#include "arch.h"
#include "asm.h"

lex_t lex;
syntax_tree_t tree;

int main (int argc, char **argv)
{
	char *fname_in, *fname_out;

	if (argc != 3) {
		printf("error: %s <fname_in> <fname_out>\n", argv[0]);
		return 1;
	}

	fname_in = argv[1];
	fname_out = argv[2];

	lex_init(&lex, fname_in);
	syntax_analyze();
	linker_generate_binary(&tree, fname_out);

	return 0;
}