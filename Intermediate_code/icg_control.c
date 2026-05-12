

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "icg.h"

/* External declarations */
extern int semantic_error_count;
extern void icg_stmt_list_generate(TreeNode *node);
extern void newLabel(char *buf);  /* Declare that newLabel exists in icg.c */

static TreeNode *child(TreeNode *node, int n) {
    if (!node || n >= node->num_children) return NULL;
    return node->children[n];
}

static TreeNode *findChild(TreeNode *node, int symbol) {
    if (!node) return NULL;
    for (int i = 0; i < node->num_children; i++)
        if (node->children[i]->symbol == symbol)
            return node->children[i];
    return NULL;
}

/* NO newLabel() function here - it's defined in icg.c */

/* Generate condition code and branch */
void icg_condition(TreeNode *node, char *true_label, char *false_label) {
    if (!node) return;
    
    TreeNode *left_exp = child(node, 0);
    TreeNode *relop = child(node, 1);
    TreeNode *right_exp = child(node, 2);
    
    if (!left_exp || !relop || !right_exp) return;
    
    char left_result[MAX_ARG], right_result[MAX_ARG];
    
    icg_expr(left_exp, left_result);
    icg_expr(right_exp, right_result);
    
    TreeNode *op_node = child(relop, 0);
    const char *op_str = "?";
    if (op_node) {
        switch (op_node->symbol) {
            case GT: op_str = ">"; break;
            case LT: op_str = "<"; break;
            case GTE: op_str = ">="; break;
            case LTE: op_str = "<="; break;
            case EQ: op_str = "=="; break;
            case NEQ: op_str = "!="; break;
            default: op_str = "?"; break;
        }
    }
    
    emit(op_str, left_result, right_result, true_label);
    emit("goto", "-", "-", false_label);
}

/* icg_if function */
void icg_if(TreeNode *node) {
    if (!node) return;
    
    if (semantic_error_count > 0) return;
    
    TreeNode *condition_node = findChild(node, CONDITION);
    TreeNode *stmt_list_node = findChild(node, STMT_LIST);
    
    if (!condition_node || !stmt_list_node) return;
    
    char true_label[MAX_ARG], false_label[MAX_ARG], end_label[MAX_ARG];
    newLabel(true_label);
    newLabel(false_label);
    newLabel(end_label);
    
    icg_condition(condition_node, true_label, false_label);
    
    emit("label", "-", "-", true_label);
    icg_stmt_list_generate(stmt_list_node);
    emit("goto", "-", "-", end_label);
    emit("label", "-", "-", false_label);
    emit("label", "-", "-", end_label);
    
    printf("[ICG] if_stmt: condition processed\n");
}

/* icg_while function */
void icg_while(TreeNode *node) {
    if (!node) return;
    
    if (semantic_error_count > 0) return;
    
    TreeNode *condition_node = findChild(node, CONDITION);
    TreeNode *stmt_list_node = findChild(node, STMT_LIST);
    
    if (!condition_node || !stmt_list_node) return;
    
    char start_label[MAX_ARG], body_label[MAX_ARG], end_label[MAX_ARG];
    newLabel(start_label);
    newLabel(body_label);
    newLabel(end_label);
    
    emit("label", "-", "-", start_label);
    icg_condition(condition_node, body_label, end_label);
    emit("label", "-", "-", body_label);
    icg_stmt_list_generate(stmt_list_node);
    emit("goto", "-", "-", start_label);
    emit("label", "-", "-", end_label);
    
    printf("[ICG] while_stmt: loop generated\n");
}