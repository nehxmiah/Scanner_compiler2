#include "parser.h"
#include "stack.h"
#include "token_map.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External arrays from parse_table.c
extern int parse_table[NUM_NON_TERMINALS][30];
extern Production productions[36];

void initParseTable();
void initProductions();

int isTerminal(int symbol) { return symbol < 100; }

const char *getSymbolName(int symbol) {
  static const char *terminals[] = {"INT",       "MAIN",
                                    "IF",        "WHILE",
                                    "PRINTF",    "ID",
                                    "NUMBER",    "STRING",
                                    "PLUS",      "MINUS",
                                    "MUL",       "DIV",
                                    "ASSIGN",    "EQ",
                                    "NEQ",       "GT",
                                    "LT",        "GTE",
                                    "LTE",       "LPAREN",
                                    "RPAREN",    "LBRACE",
                                    "RBRACE",    "SEMICOLON",
                                    "COMMA",     "LEXICAL_ERROR",
                                    "END_MARKER"};

  static const char *non_terminals[] = {
      "PROGRAM",   "STMT_LIST", "STMT_LIST_P", "STMT",    "DECL_STMT",
      "ID_LIST",   "ID_LIST_P", "ASSIGN_STMT", "EXP",     "EXP_P",
      "TERM",      "TERM_P",    "FACTOR",      "IF_STMT", "WHILE_STMT",
      "CONDITION", "RELOP",     "PRINT_STMT"};

  if (symbol < 100) {
    return terminals[symbol];
  } else if (symbol >= 100 && symbol < 100 + NUM_NON_TERMINALS) {
    return non_terminals[symbol - 100];
  }
  return "UNKNOWN";
}

void parse(FILE *fp) {
  Stack stack;
  initStack(&stack);

  TreeNode *root = createNode(PROGRAM, 0);

  initProductions();
  initParseTable();

  push(&stack, END_MARKER, NULL);
  push(&stack, PROGRAM, root);

  Token token = getNextToken(fp);
  int step = 0;
  int error_flag = 0;
  int lexical_error_count = 0;
  int syntax_error_count = 0;

  printf("--- Beginning LL(1) Parse with Error Reporting ---\n");

  while (!isEmpty(&stack)) {
    StackItem topItem = peek(&stack);
    int top = topItem.symbol;
    TreeNode *currentNode = topItem.node;
    step++;

    // ---------------------------------------------------------
    // CHECK FOR LEXICAL ERRORS FIRST
    // ---------------------------------------------------------
    if (token.type == LEXICAL_ERROR) {
      printf("[%03d] LEXICAL ERROR DETECTED: Token '%s' is invalid in this "
             "context.\n",
             step, token.lexeme);
      printf("      ACTION: Skipping invalid token and continuing parse...\n");
      lexical_error_count++;
      error_flag = 1;
      token = getNextToken(fp);
      continue;
    }

    // ---------------------------------------------------------
    // CASE 1: Top of stack is END_MARKER ($)
    // ---------------------------------------------------------
    if (top == END_MARKER) {
      if (token.type == END_MARKER) {
        printf("[%03d] SUCCESS: Reached end of input. Parse complete.\n", step);
        pop(&stack);
      } else {
        printf(
            "[%03d] SYNTAX ERROR: Expected end of file, but found '%s' (%s).\n",
            step, token.lexeme, getSymbolName(token.type));
        printf("      HINT: Remove extra tokens after end of program.\n");
        printf("      ACTION: Consuming extra token and forcing accept.\n");
        token = getNextToken(fp);
        error_flag = 1;
        syntax_error_count++;
      }
    }

    // ---------------------------------------------------------
    // CASE 2 & 3: Top of stack is a Terminal
    // ---------------------------------------------------------
    else if (isTerminal(top)) {
      if (top == token.type) {
        // CASE 2: Match -> Pop stack, advance input
        printf("[%03d] MATCH: Expected '%s', found '%s'.\n", step,
               getSymbolName(top), token.lexeme);

        if (currentNode) {
          strcpy(currentNode->lexeme, token.lexeme);
        }

        pop(&stack);
        token = getNextToken(fp);
      } else {
        // CASE 3: Mismatch -> Error and Panic Mode Recovery
        printf("[%03d] SYNTAX ERROR: Terminal mismatch.\n", step);
        printf("      Expected: '%s' (%s)\n", getSymbolName(top),
               getSymbolName(top));
        printf("      Found:    '%s' (%s)\n", token.lexeme,
               getSymbolName(token.type));
        printf("      HINT: Check for missing or incorrect "
               "punctuation/operators.\n");
        printf("      ACTION: Popping expected token from stack and "
               "continuing.\n");
        pop(&stack);
        token = getNextToken(fp); // FIX: Advance token to prevent infinite loop
        error_flag = 1;
        syntax_error_count++;
      }
    }

    // ---------------------------------------------------------
    // CASE 4: Top of stack is a Non-Terminal
    // ---------------------------------------------------------
    else {
      int prod_index = parse_table[top][token.type];

      if (prod_index != -1) {
        Production p = productions[prod_index];

        printf("[%03d] EXPAND: %s -> ", step, getSymbolName(top));
        if (p.rhs_len == 0) {
          printf("ε (epsilon)\n");
          // Don't create epsilon nodes - just skip
        } else {
          for (int i = 0; i < p.rhs_len; i++) {
            printf("%s ", getSymbolName(p.rhs[i]));
          }
          printf("\n");
        }

        pop(&stack);

        // Only create children for non-epsilon productions
        if (p.rhs_len > 0) {
          TreeNode *children[10];
          for (int i = 0; i < p.rhs_len; i++) {
            int child_sym = p.rhs[i];
            int is_term = isTerminal(child_sym);
            children[i] = createNode(child_sym, is_term);
            addChild(currentNode, children[i]);
          }

          for (int i = p.rhs_len - 1; i >= 0; i--) {
            push(&stack, p.rhs[i], children[i]);
          }
        }
      } else {
        // No table entry -> Error and Panic Mode Recovery
        printf("[%03d] SYNTAX ERROR: Unexpected token '%s' (%s).\n", step,
               token.lexeme, getSymbolName(token.type));
        printf("      While parsing: %s\n", getSymbolName(top));
        printf("      Expected one of: ");

        // Show what tokens were expected (lookahead set)
        int first = 1;
        for (int i = 0; i < 30; i++) {
          if (parse_table[top][i] != -1) {
            if (!first)
              printf(", ");
            printf("%s", getSymbolName(i));
            first = 0;
          }
        }
        printf("\n");
        printf("      HINT: Check syntax for %s construct.\n",
               getSymbolName(top));

        syntax_error_count++;
        error_flag = 1;

        // PANIC MODE with feedback
        printf("      Entering PANIC MODE: Skipping tokens to find "
               "synchronization point...\n");

        int recovered = 0;
        int skip_count = 0;
        while (token.type != END_MARKER && token.type != LEXICAL_ERROR) {
          token = getNextToken(fp);
          skip_count++;
          if (parse_table[top][token.type] != -1) {
            recovered = 1;
            printf("      RECOVERY: Found valid token '%s' after skipping %d "
                   "tokens.\n",
                   token.lexeme, skip_count);
            printf("      Resuming parse with %s -> %s...\n",
                   getSymbolName(top), getSymbolName(token.type));
            break;
          }
        }

        if (!recovered) {
          printf("      RECOVERY FAILED: Reached EOF without finding sync "
                 "token.\n");
          printf("      ACTION: Popping %s from stack to continue.\n",
                 getSymbolName(top));
          pop(&stack);
        }
      }
    }
  }

  printf("------------------------------------------------\n");
  printf("PARSING SUMMARY:\n");
  printf("  Total Steps: %d\n", step);
  printf("  Lexical Errors: %d\n", lexical_error_count);
  printf("  Syntax Errors: %d\n", syntax_error_count);

  if (error_flag) {
    printf("  Status: FAILED with errors.\n");
    if (lexical_error_count > 0) {
      printf("  NOTE: Fix lexical errors (invalid characters/tokens) first.\n");
    }
    if (syntax_error_count > 0) {
      printf("  NOTE: Review grammar rules for the reported non-terminals.\n");
    }
  } else {
    printf("  Status: SUCCESS - No errors detected!\n");
    printf("\n========== PARSE TREE ==========\n");
    printTree(root, 0);
    printf("================================\n");
  }

  freeTree(root);
}
