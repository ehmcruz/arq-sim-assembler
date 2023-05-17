#ifndef __LINKER_H__
#define __LINKER_H__

#include "lex.h"
#include "syntax.h"

void linker_generate_binary (syntax_tree_t *tree, char *fname_out);

#endif
