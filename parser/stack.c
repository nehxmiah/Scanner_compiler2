#include "stack.h"
#include <stdio.h>

void initStack(Stack *s) { s->top = -1; }

void push(Stack *s, int symbol, TreeNode *node) {
  s->top++;
  s->items[s->top].symbol = symbol;
  s->items[s->top].node = node;
}

StackItem pop(Stack *s) { return s->items[s->top--]; }

StackItem peek(Stack *s) { return s->items[s->top]; }

int isEmpty(Stack *s) { return s->top == -1; }
