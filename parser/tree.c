#include "tree.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TreeNode *createNode(int symbol, int is_terminal) {
  TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
  node->symbol = symbol;
  node->is_terminal = is_terminal;
  node->num_children = 0;
  node->lexeme[0] = '\0';
  return node;
}

void addChild(TreeNode *parent, TreeNode *child) {
  if (parent->num_children < 10) {
    parent->children[parent->num_children++] = child;
  }
}

const char *getNodeSymbolName(int symbol) {
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

void printTree(TreeNode *root, int indent) {
  if (!root)
    return;

  // Skip epsilon nodes (symbol == -1 or lexeme == "ε")
  if (root->symbol == -1 || strcmp(root->lexeme, "ε") == 0) {
    return; // Don't print epsilon
  }

  for (int i = 0; i < indent; i++)
    printf("│   ");

  if (root->is_terminal) {
    if (root->symbol == LEXICAL_ERROR) {
      printf("├── ERROR: '%s'\n", root->lexeme);
    } else {
      // Show symbol with lexeme (e.g., PLUS = "+")
      printf("├── %s = \"%s\"\n", getNodeSymbolName(root->symbol),
             root->lexeme);
    }
  } else {
    // Non-terminal - just show name
    printf("├── %s\n", getNodeSymbolName(root->symbol));
  }

  for (int i = 0; i < root->num_children; i++) {
    printTree(root->children[i], indent + 1);
  }
}

void freeTree(TreeNode *root) {
  if (!root)
    return;
  for (int i = 0; i < root->num_children; i++) {
    freeTree(root->children[i]);
  }
  free(root);
}
