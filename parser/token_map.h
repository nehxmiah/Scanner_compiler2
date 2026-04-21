#ifndef TOKEN_MAP_H
#define TOKEN_MAP_H

#include "parser.h"

typedef struct {
  Terminal type;
  char lexeme[100];
} Token;

Token getNextToken(FILE *fp);

#endif
