#include "token_map.h"
#include <stdio.h>
#include <string.h>

Token getNextToken(FILE *fp) {
  Token t;
  char token_name[100];
  char lexeme[100];
  char category[100];
  char buffer[512];
  int line;

  static int header_skipped = 0;

  // Skip header lines on first read
  if (!header_skipped) {
    fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);
    header_skipped = 1;
  }

  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    // Parse the token line: LINE LEXEME TOKEN_NAME CATEGORY
    if (sscanf(buffer, "%d %99s %99s %99s", &line, lexeme, token_name,
               category) != 4) {
      fprintf(stderr, "[LEXICAL ERROR] Malformed token line in input: %s",
              buffer);
      continue;
    }

    // DO NOT SKIP ERRORS - Return them as tokens so parser can report them
    if (strcmp(token_name, "LEXICAL_ERROR") == 0) {
      t.type = LEXICAL_ERROR;
      strcpy(t.lexeme, lexeme);
      fprintf(stderr,
              "[LEXICAL ERROR] Line %d: Invalid token '%s' (Category: %s)\n",
              line, lexeme, category);
      return t;
    }

    // Map valid tokens to Terminal types
    if (strcmp(token_name, "KEYWORD_INT") == 0) {
      t.type = INT;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "KEYWORD_MAIN") == 0) {
      t.type = MAIN;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "KEYWORD_IF") == 0) {
      t.type = IF;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "KEYWORD_WHILE") == 0) {
      t.type = WHILE;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "KEYWORD_PRINTF") == 0) {
      t.type = PRINTF;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "IDENTIFIER") == 0) {
      t.type = ID;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "INTEGER_LITERAL") == 0) {
      t.type = NUMBER;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "STRING_LITERAL") == 0) {
      t.type = STRING;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_PLUS") == 0) {
      t.type = PLUS;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_MINUS") == 0) {
      t.type = MINUS;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_MULTIPLICATION") == 0) {
      t.type = MUL;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_DIVISION") == 0) {
      t.type = DIV;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_ASSIGN") == 0) {
      t.type = ASSIGN;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_EQUAL") == 0) {
      t.type = EQ;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_NOT_EQUAL") == 0) {
      t.type = NEQ;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_GREATER_THAN") == 0) {
      t.type = GT;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_LESS_THAN") == 0) {
      t.type = LT;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_GREATER_EQUAL") == 0) {
      t.type = GTE;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "OPERATOR_LESS_EQUAL") == 0) {
      t.type = LTE;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "LEFT_PARENTHESIS") == 0) {
      t.type = LPAREN;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "RIGHT_PARENTHESIS") == 0) {
      t.type = RPAREN;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "LEFT_BRACKETS") == 0) {
      t.type = LBRACE;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "RIGHT_BRACKETS") == 0) {
      t.type = RBRACE;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "SEMICOLON") == 0) {
      t.type = SEMICOLON;
      strcpy(t.lexeme, lexeme);
      return t;
    } else if (strcmp(token_name, "COMMA") == 0) {
      t.type = COMMA;
      strcpy(t.lexeme, lexeme);
      return t;
    } else {
      // Unknown token type - report as error
      t.type = LEXICAL_ERROR;
      strcpy(t.lexeme, lexeme);
      fprintf(
          stderr,
          "[LEXICAL ERROR] Line %d: Unknown token type '%s' for lexeme '%s'\n",
          line, token_name, lexeme);
      return t;
    }
  }

  // End of file
  t.type = END_MARKER;
  strcpy(t.lexeme, "$");
  return t;
}
