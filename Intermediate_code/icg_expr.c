/*
 * icg_expr.c
 * Expression & Assignment Intermediate Code Generation
 * Owner: Ivy Mokeira
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "icg.h"

extern int semantic_error_count;

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

static int semantic_check_div_zero(const char *arg2) {
    if (strcmp(arg2, "0") == 0) {
        fprintf(stderr, "[SEMANTIC WARNING] Compile-time division by zero detected.\n");
        return 1;
    }
    return 0;
}

static void semantic_check_uninitialised(const char *name) {
    int idx = sym_lookup(name);
    if (idx >= 0 && !sym_table.entries[idx].initialised) {
        fprintf(stderr, "[SEMANTIC WARNING] Variable '%s' may be used before assignment.\n", name);
    }
}

static const char *get_operator_string(int symbol) {
    switch (symbol) {
        case PLUS:  return "+";
        case MINUS: return "-";
        case MUL:   return "*";
        case DIV:   return "/";
        case GT:    return ">";
        case LT:    return "<";
        case EQ:    return "==";
        case NEQ:   return "!=";
        case GTE:   return ">=";
        case LTE:   return "<=";
        default:    return "?";
    }
}

static void icg_factor(TreeNode *node, char *result_out) {
    if (!node) { strcpy(result_out, "-"); return; }

    TreeNode *id_node = findChild(node, ID);
    if (id_node) {
        int idx = sym_lookup(id_node->lexeme);
        if (idx >= 0)
            semantic_check_uninitialised(id_node->lexeme);
        strcpy(result_out, id_node->lexeme);
        return;
    }

    TreeNode *num_node = findChild(node, NUMBER);
    if (num_node) {
        strcpy(result_out, num_node->lexeme);
        return;
    }

    TreeNode *lparen_node = findChild(node, LPAREN);
    if (lparen_node) {
        TreeNode *inner_exp = NULL;
        for (int i = 0; i < node->num_children; i++) {
            if (node->children[i]->symbol == EXP) {
                inner_exp = node->children[i];
                break;
            }
        }
        if (inner_exp) {
            icg_expr(inner_exp, result_out);
            return;
        }
    }

    TreeNode *exp_node = findChild(node, EXP);
    if (exp_node) {
        icg_expr(exp_node, result_out);
        return;
    }

    strcpy(result_out, "-");
}

static void icg_term_p(TreeNode *node, const char *left, char *result_out) {
    if (!node || node->num_children == 0) {
        strcpy(result_out, left);
        return;
    }

    TreeNode *op_node     = child(node, 0);
    TreeNode *factor_node = child(node, 1);
    TreeNode *term_p_node = child(node, 2);

    char right[MAX_ARG];
    icg_factor(factor_node, right);

    if (op_node && op_node->symbol == DIV) {
        if (semantic_check_div_zero(right)) {
            strcpy(result_out, "-");
            return;
        }
    }

    char temp[MAX_ARG];
    newTemp(temp);
    const char *op_str = op_node ? get_operator_string(op_node->symbol) : "?";
    emit(op_str, left, right, temp);

    icg_term_p(term_p_node, temp, result_out);
}

static void icg_term(TreeNode *node, char *result_out) {
    if (!node) { strcpy(result_out, "-"); return; }

    TreeNode *factor_node = child(node, 0);
    TreeNode *term_p_node = child(node, 1);

    char factor_result[MAX_ARG];
    icg_factor(factor_node, factor_result);
    icg_term_p(term_p_node, factor_result, result_out);
}

static void icg_exp_p(TreeNode *node, const char *left, char *result_out) {
    if (!node || node->num_children == 0) {
        strcpy(result_out, left);
        return;
    }

    TreeNode *op_node    = child(node, 0);
    TreeNode *term_node  = child(node, 1);
    TreeNode *exp_p_node = child(node, 2);

    char right[MAX_ARG];
    icg_term(term_node, right);

    char temp[MAX_ARG];
    newTemp(temp);
    const char *op_str = op_node ? get_operator_string(op_node->symbol) : "?";
    emit(op_str, left, right, temp);

    icg_exp_p(exp_p_node, temp, result_out);
}

void icg_expr(TreeNode *node, char *result_out) {
    if (!node) { strcpy(result_out, "-"); return; }

    if (node->symbol == CONDITION) {
        TreeNode *left_exp  = child(node, 0);
        TreeNode *relop     = child(node, 1);
        TreeNode *right_exp = child(node, 2);

        char left[MAX_ARG], right[MAX_ARG];
        icg_expr(left_exp, left);
        icg_expr(right_exp, right);

        const char *op_str = "?";
        if (relop && relop->num_children > 0)
            op_str = get_operator_string(child(relop, 0)->symbol);

        char temp[MAX_ARG];
        newTemp(temp);
        emit(op_str, left, right, temp);
        strcpy(result_out, temp);
        return;
    }

    if (node->symbol == EXP) {
        TreeNode *term_node  = child(node, 0);
        TreeNode *exp_p_node = child(node, 1);

        char term_result[MAX_ARG];
        icg_term(term_node, term_result);
        icg_exp_p(exp_p_node, term_result, result_out);
        return;
    }

    icg_factor(node, result_out);
}

/* ─────────────────────────────────────────────────────────────────
 * icg_assign_check - PASS 1: Check for errors, don't generate code
 * ───────────────────────────────────────────────────────────────── */
void icg_assign_check(TreeNode *node) {
    if (!node) return;

    TreeNode *id_node  = child(node, 0);
    TreeNode *exp_node = child(node, 2);

    if (!id_node || !exp_node) return;

    int idx = sym_lookup(id_node->lexeme);
    if (idx < 0) return;

    char exp_result[MAX_ARG];
    icg_expr(exp_node, exp_result);

    if (exp_result[0] == '"') {
        fprintf(stderr, "[SEMANTIC ERROR] Type mismatch: cannot assign string to '%s'.\n", id_node->lexeme);
        semantic_error_count++;
        return;
    }

    if (strcmp(exp_result, "-") == 0) {
        fprintf(stderr, "[SEMANTIC ERROR] Invalid expression for '%s'.\n", id_node->lexeme);
        semantic_error_count++;
        return;
    }

    /* Mark the target variable as initialized for later semantic checks. */
    sym_update(id_node->lexeme);
}

/* ─────────────────────────────────────────────────────────────────
 * icg_assign_generate - PASS 2: Actually generate code (only if no errors)
 * ───────────────────────────────────────────────────────────────── */
void icg_assign_generate(TreeNode *node) {
    if (!node) return;

    TreeNode *id_node  = child(node, 0);
    TreeNode *exp_node = child(node, 2);

    if (!id_node || !exp_node) return;

    char exp_result[MAX_ARG];
    icg_expr(exp_node, exp_result);

    emit("=", exp_result, "-", id_node->lexeme);
    sym_update(id_node->lexeme);
    printf("[ICG] assign_stmt: %s = %s\n", id_node->lexeme, exp_result);
}