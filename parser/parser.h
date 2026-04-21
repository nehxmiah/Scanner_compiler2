#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#define MAX_RHS 10

// -------------------- TERMINALS --------------------
typedef enum {
  INT,
  MAIN,
  IF,
  WHILE,
  PRINTF,
  ID,
  NUMBER,
  STRING,
  PLUS,
  MINUS,
  MUL,
  DIV,
  ASSIGN,
  EQ,
  NEQ,
  GT,
  LT,
  GTE,
  LTE,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  SEMICOLON,
  COMMA,
  LEXICAL_ERROR, // ADD THIS - for invalid tokens from scanner
  END_MARKER
} Terminal;

// -------------------- NON-TERMINALS --------------------
typedef enum {
  PROGRAM = 100,
  STMT_LIST,
  STMT_LIST_P,
  STMT,
  DECL_STMT,
  ID_LIST,
  ID_LIST_P,
  ASSIGN_STMT,
  EXP,
  EXP_P,
  TERM,
  TERM_P,
  FACTOR,
  IF_STMT,
  WHILE_STMT,
  CONDITION,
  RELOP,
  PRINT_STMT,
  NUM_NON_TERMINALS
} NonTerminal;

// -------------------- PRODUCTION STRUCT --------------------
typedef struct {
  int lhs;
  int rhs[MAX_RHS];
  int rhs_len;
} Production;

// -------------------- FUNCTION DECLARATIONS --------------------
void initProductions();
void initParseTable();
void parse(FILE *fp);

#endif
