#ifndef STACK_H
#define STACK_H

#include "tree.h"

typedef struct {
  int symbol;
  TreeNode *node;
} StackItem;

typedef struct {
  StackItem items[100];
  int top;
} Stack;

void initStack(Stack *s);
void push(Stack *s, int symbol, TreeNode *node);
StackItem pop(Stack *s);
StackItem peek(Stack *s);
int isEmpty(Stack *s);

#endif
