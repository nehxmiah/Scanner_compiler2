#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "icg.h"

/* External declarations */
extern int semantic_error_count;
extern void icg_stmt_list_generate(TreeNode *node);

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

static int structural_check_condition(TreeNode *cond_node, const char *construct) {
    if (!cond_node) {
        fprintf(stderr, "[STRUCTURAL ERROR] %s statement has missing condition.\n", construct);
        return 0;
    }
    if (cond_node->num_children >= 3) {
        TreeNode *l_exp = child(cond_node, 0);
        TreeNode *r_exp = child(cond_node, 2);
        int l_const = (l_exp && l_exp->num_children > 0 &&
                       child(child(child(l_exp, 0), 0), 0) &&
                       child(child(child(l_exp, 0), 0), 0)->symbol == NUMBER);
        int r_const = (r_exp && r_exp->num_children > 0 &&
                       child(child(child(r_exp, 0), 0), 0) &&
                       child(child(child(r_exp, 0), 0), 0)->symbol == NUMBER);
        if (l_const && r_const && strcmp(construct, "while") == 0) {
            fprintf(stderr, "[STRUCTURAL WARNING] while loop has constant condition - infinite loop risk.\n");
        }
    }
    return 1;
}

/* icg_if: allocates ONE label (end), uses if_false to jump over body */
void icg_if(TreeNode *node) {
    if (!node) return;
    if (semantic_error_count > 0) return;

    TreeNode *cond = findChild(node, CONDITION);
    TreeNode *body = findChild(node, STMT_LIST);

    if (!structural_check_condition(cond, "if")) return;

    char cond_temp[MAX_ARG], L_end[MAX_ARG];
    icg_expr(cond, cond_temp);
    newLabel(L_end);

    emit("if_false", cond_temp, "-", L_end);

    sym_enter_scope();
    if (body) icg_stmt_list_generate(body);
    sym_exit_scope();

    emit("label", "-", "-", L_end);

    printf("[ICG] if_stmt: condition=%s  end_label=%s\n", cond_temp, L_end);
}

/* icg_while: allocates TWO labels (start, end), uses if_false to exit loop */
void icg_while(TreeNode *node) {
    if (!node) return;
    if (semantic_error_count > 0) return;

    TreeNode *cond = findChild(node, CONDITION);
    TreeNode *body = findChild(node, STMT_LIST);

    if (!structural_check_condition(cond, "while")) return;

    char L_start[MAX_ARG], L_end[MAX_ARG], cond_temp[MAX_ARG];
    newLabel(L_start);
    newLabel(L_end);

    emit("label", "-", "-", L_start);
    icg_expr(cond, cond_temp);
    emit("if_false", cond_temp, "-", L_end);

    sym_enter_scope();
    if (body) icg_stmt_list_generate(body);
    sym_exit_scope();

    emit("goto", "-", "-", L_start);
    emit("label", "-", "-", L_end);

    printf("[ICG] while_stmt: condition=%s  start=%s  end=%s\n", cond_temp, L_start, L_end);
}