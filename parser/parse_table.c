#include "parser.h"

#define ERROR -1

// total productions = 36 (0–35)
Production productions[36];

// Parse table: rows are for non-terminals (offset by 100), columns for
// terminals
int parse_table[120][30];

void initProductions() {

  productions[0] = (Production){
      PROGRAM, {INT, MAIN, LPAREN, RPAREN, LBRACE, STMT_LIST, RBRACE}, 7};

  productions[1] = (Production){STMT_LIST, {STMT, STMT_LIST_P}, 2};

  productions[2] = (Production){STMT_LIST_P, {STMT, STMT_LIST_P}, 2};
  productions[3] = (Production){STMT_LIST_P, {}, 0}; // ε

  productions[4] = (Production){STMT, {DECL_STMT}, 1};
  productions[5] = (Production){STMT, {ASSIGN_STMT}, 1};
  productions[6] = (Production){STMT, {IF_STMT}, 1};
  productions[7] = (Production){STMT, {WHILE_STMT}, 1};
  productions[8] = (Production){STMT, {PRINT_STMT}, 1};
  productions[9] = (Production){STMT, {SEMICOLON}, 1};

  productions[10] = (Production){DECL_STMT, {INT, ID_LIST, SEMICOLON}, 3};

  productions[11] = (Production){ID_LIST, {ID, ID_LIST_P}, 2};
  productions[12] = (Production){ID_LIST_P, {COMMA, ID, ID_LIST_P}, 3};
  productions[13] = (Production){ID_LIST_P, {}, 0}; // ε

  productions[14] = (Production){ASSIGN_STMT, {ID, ASSIGN, EXP, SEMICOLON}, 4};

  productions[15] = (Production){EXP, {TERM, EXP_P}, 2};
  productions[16] = (Production){EXP_P, {PLUS, TERM, EXP_P}, 3};
  productions[17] = (Production){EXP_P, {MINUS, TERM, EXP_P}, 3};
  productions[18] = (Production){EXP_P, {}, 0}; // ε

  productions[19] = (Production){TERM, {FACTOR, TERM_P}, 2};
  productions[20] = (Production){TERM_P, {MUL, FACTOR, TERM_P}, 3};
  productions[21] = (Production){TERM_P, {DIV, FACTOR, TERM_P}, 3};
  productions[22] = (Production){TERM_P, {}, 0}; // ε

  productions[23] = (Production){FACTOR, {ID}, 1};
  productions[24] = (Production){FACTOR, {NUMBER}, 1};
  productions[25] = (Production){FACTOR, {LPAREN, EXP, RPAREN}, 3};

  productions[26] = (Production){
      IF_STMT, {IF, LPAREN, CONDITION, RPAREN, LBRACE, STMT_LIST, RBRACE}, 7};

  productions[27] = (Production){
      WHILE_STMT,
      {WHILE, LPAREN, CONDITION, RPAREN, LBRACE, STMT_LIST, RBRACE},
      7};

  productions[28] = (Production){CONDITION, {EXP, RELOP, EXP}, 3};

  productions[29] = (Production){RELOP, {GT}, 1};
  productions[30] = (Production){RELOP, {LT}, 1};
  productions[31] = (Production){RELOP, {GTE}, 1};
  productions[32] = (Production){RELOP, {LTE}, 1};
  productions[33] = (Production){RELOP, {EQ}, 1};
  productions[34] = (Production){RELOP, {NEQ}, 1};

  productions[35] = (Production){
      PRINT_STMT, {PRINTF, LPAREN, STRING, COMMA, ID, RPAREN, SEMICOLON}, 7};
}

void initParseTable() {

  // Initialize all entries to ERROR
  for (int i = 0; i < 120; i++)
    for (int j = 0; j < 30; j++)
      parse_table[i][j] = ERROR;

  // PROGRAM (100)
  parse_table[100][INT] = 0;

  // STMT_LIST (101)
  parse_table[101][INT] = 1;
  parse_table[101][ID] = 1;
  parse_table[101][IF] = 1;
  parse_table[101][WHILE] = 1;
  parse_table[101][PRINTF] = 1;
  parse_table[101][SEMICOLON] = 1;

  // STMT_LIST' (102)
  parse_table[102][INT] = 2;
  parse_table[102][ID] = 2;
  parse_table[102][IF] = 2;
  parse_table[102][WHILE] = 2;
  parse_table[102][PRINTF] = 2;
  parse_table[102][SEMICOLON] = 2;
  parse_table[102][RBRACE] = 3;

  // STMT (103)
  parse_table[103][INT] = 4;
  parse_table[103][ID] = 5;
  parse_table[103][IF] = 6;
  parse_table[103][WHILE] = 7;
  parse_table[103][PRINTF] = 8;
  parse_table[103][SEMICOLON] = 9;

  // DECL (104)
  parse_table[104][INT] = 10;

  // ID_LIST (105)
  parse_table[105][ID] = 11;

  // ID_LIST_P (106)
  parse_table[106][COMMA] = 12;
  parse_table[106][SEMICOLON] = 13;

  // ASSIGN_STMT (107)
  parse_table[107][ID] = 14;

  // EXP (108)
  parse_table[108][ID] = 15;
  parse_table[108][NUMBER] = 15;
  parse_table[108][LPAREN] = 15;

  // EXP' (109)
  parse_table[109][PLUS] = 16;
  parse_table[109][MINUS] = 17;
  parse_table[109][SEMICOLON] = 18;
  parse_table[109][RPAREN] = 18;
  parse_table[109][GT] = 18;
  parse_table[109][LT] = 18;
  parse_table[109][EQ] = 18;
  parse_table[109][NEQ] = 18;
  parse_table[109][GTE] = 18;
  parse_table[109][LTE] = 18;

  // TERM (110)
  parse_table[110][ID] = 19;
  parse_table[110][NUMBER] = 19;
  parse_table[110][LPAREN] = 19;

  // TERM_P (111)
  parse_table[111][MUL] = 20;
  parse_table[111][DIV] = 21;
  parse_table[111][PLUS] = 22;
  parse_table[111][MINUS] = 22;
  parse_table[111][SEMICOLON] = 22;
  parse_table[111][RPAREN] = 22;
  parse_table[111][GT] = 22;
  parse_table[111][LT] = 22;
  parse_table[111][EQ] = 22;
  parse_table[111][NEQ] = 22;
  parse_table[111][GTE] = 22;
  parse_table[111][LTE] = 22;

  // FACTOR (112)
  parse_table[112][ID] = 23;
  parse_table[112][NUMBER] = 24;
  parse_table[112][LPAREN] = 25;

  // IF (113)
  parse_table[113][IF] = 26;

  // WHILE (114)
  parse_table[114][WHILE] = 27;

  // CONDITION (115)
  parse_table[115][ID] = 28;
  parse_table[115][NUMBER] = 28;
  parse_table[115][LPAREN] = 28;

  // RELOP (116)
  parse_table[116][GT] = 29;
  parse_table[116][LT] = 30;
  parse_table[116][GTE] = 31;
  parse_table[116][LTE] = 32;
  parse_table[116][EQ] = 33;
  parse_table[116][NEQ] = 34;

  // PRINT (117)
  parse_table[117][PRINTF] = 35;
}
