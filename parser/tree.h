#ifndef TREE_H
#define TREE_H

typedef struct TreeNode {
  int symbol;       // Terminal or Non-terminal code
  char lexeme[100]; // For terminals: the actual text (e.g., "x", "10")
  int is_terminal;  // 1 if terminal, 0 if non-terminal
  int num_children; // Number of children
  struct TreeNode
      *children[10]; // Max 10 children per node (sufficient for your grammar)
} TreeNode;

TreeNode *createNode(int symbol, int is_terminal);
void addChild(TreeNode *parent, TreeNode *child);
void printTree(TreeNode *root, int indent);
void freeTree(TreeNode *root);
const char *getNodeSymbolName(int symbol); // Helper for printing

#endif
